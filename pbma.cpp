#include "pbma.h"

#include <algorithm>
#include <climits>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <memory>
#include <numeric>
#include <sstream>
#include <thread>

// @PBMA: Das müssen Sie alles nicht lesen oder verstehen, nur verwenden.

using namespace std;

static constexpr long MAX_FILESIZE = 1073741824; // one GByte, for binary

pbma_exception::pbma_exception(const std::string& _cause) {
    this->cause = "pbma exception:: " + _cause;
}

pbma_exception::pbma_exception(const std::string& _cause, int val) {
    this->cause = "pbma exception:: " + _cause + " : " + to_string(val);
}

pbma_exception::pbma_exception(const std::string& _cause, long val) {
    this->cause = "pbma exception:: " + _cause + " : " + to_string(val);
}

pbma_exception::pbma_exception(const std::string& _cause,
                               const std::string& val) {
    this->cause = "pbma exception:: " + _cause + " : " + val;
}

const char* pbma_exception::what() const noexcept {
    return cause.c_str();
}

// internal, called on error
static pbma_exception error(const std::string& _cause) {
    cerr << "ERROR: " << _cause << endl; // ensure something is printed
    return pbma_exception(_cause);
}

// internal, error formatting
// stackoverflow.com/questions/2342162/stdstring-formatting-like-sprintf
// c++14 way, still ugly that the caller needs to use .c_str() on strings
// Worse: clangd complains bitterly about using a nonliteral as first argument
// in both uses of snprintf. This is wrong in the first use and no problem
// in the second use, as this is intended. However, the warning
// "-Wformat-nonliteral" is default-enabled in automated cland environments
// (which we might be forced to use someday). Thus, we use rather a horribly and
// ugly trick to inhibt the usage of snprintf to the compiler in order to
// inhibit the warning and to not confuse unexpecting students.
extern "C" {
typedef int (* snprintf_t)(char * str, size_t size,
                           const char * format, ...);
snprintf_t my_secret_snprintf = snprintf;
}
template <typename... Args>
static string format(const std::string& fmt, Args... args) {
    const char* cfmt = fmt.c_str();
    // +1 to accomodate extra space for '\0'
    size_t size = static_cast<size_t>(my_secret_snprintf(nullptr, 0, cfmt, args...) + 1);
    auto buf = make_unique<char[]>(size);
    my_secret_snprintf(buf.get(), size, cfmt, args...);
    // We don't want the '\0' inside the returned string
    return string(buf.get(), buf.get() + size - 1);
}

bool starts_with(const std::string& s, const std::string& prefix) {
    return equal(prefix.begin(), prefix.end(), s.begin());
}

std::string format(long val, int length, char fill_char) {
    std::ostringstream ostr;
    if (val < 0) { // if negative handle output of '-' and count it
        ostr << '-';
        if (length > 0) {
            length -= 1;
        }
        val *= -1;
    }
    ostr << std::setfill(fill_char) << std::setw(length) << val;
    return ostr.str();
}

// -- general utilities --

// -- file reading --

bool file_exists(const std::string& filename) {
    ifstream f(filename.c_str()); // automatically closed
    return f.good();
}

vector<char> read_bytes(const std::string& filename) {
    ifstream is(filename, ifstream::binary);
    if (!is) {
        const char* s = filename.c_str();
        throw error(format("read_bytes: filename=%s nicht lesbar", s));
    }
    // go to end of file
    is.seekg(0, is.end);
    const streamsize length = is.tellg(); // length/size of file
    // go to start of file
    is.seekg(0, is.beg);
    if (length > MAX_FILESIZE || length < 0) {
        const char* s = filename.c_str();
        throw error(format("read_bytes: filename=%s zu gross", s));
    }
    vector<char> buffer(static_cast<size_t>(length));
    is.read(buffer.data(), length); // read completely
    if (!is.good()) {
        const char* s = filename.c_str();
        throw error(format("read_bytes: filename=%s Einlesefehler", s));
    }
    is.close();
    return buffer; // should not be copied, move semantics
}

static const char COMMENT_PREFIX[] = "#";
vector<string> read_lines(const std::string& filename) {
    ifstream is(filename);
    if (!is) {
        const char* s = filename.c_str();
        throw error(format("read_lines: filename=%s nicht lesbar", s));
    }
    vector<string> lines;
    string line;
    while (getline(is, line)) {
        if (line.empty()) {
            continue;
        }
        if (starts_with(line, COMMENT_PREFIX)) {
            continue;
        }
        lines.push_back(line);
    }
    return lines;
}

// unfortunately, regular expressions let compile time explode
// thus, we parse manually, which is awful
static const char CHARS_DELIM[] = " \t,;'\"\0"; // what are delimiters
static const char CHARS_GERMAN_WORDS[] = "abcdefghijklmnopqrstuvwxyz"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "0123456789äöüßÄÖÜ"; // what may remain
static const char CHARS_INTEGRAL_WORDS[] = "-+0123456789";

// private, words as changeable parameter, for reuse
static void read_accept_str_vec(const string& line, const string& accept,
                                vector<string>& words) {
    size_t prev = 0, pos;
    while ((pos = line.find_first_of(CHARS_DELIM, prev)) != string::npos) {
        if (pos > prev) {
            string word = line.substr(prev, pos - prev);
            if (word.size() > 0 && // nonempty, only of acceptable chars
                word.find_first_not_of(accept) == string::npos) {
                words.push_back(word);
            }
        }
        prev = pos + 1;
    }
    // something remaining, do not forget last match
    string word = line.substr(prev);
    // double that code of the if in the loop
    if (word.size() > 0 && // nonempty, only of acceptable chars
        word.find_first_not_of(accept) == string::npos) {
        words.push_back(word);
    }
}

static vector<string> read_accept_str(const string& line,
                                      const string& accept) {
    vector<string> words;
    read_accept_str_vec(line, accept, words);
    return words;
}

static vector<string> read_accept_file(const string& filename,
                                       const string& accept) {
    vector<string> words;
    for (const string& line : read_lines(filename)) {
        read_accept_str_vec(line, accept, words);
    }
    return words;
}

vector<string> read_words(const std::string& filename) {
    return read_accept_file(filename, CHARS_GERMAN_WORDS);
}

vector<int> read_ints(const std::string& filename) {
    vector<int> ints;
    for (const string& iword :
             read_accept_file(filename, CHARS_INTEGRAL_WORDS)) {
        try {
            const int val = stoi(iword);
            ints.push_back(val);
        } catch (const invalid_argument& err) {
            auto s = iword.c_str();
            throw error(format("read_ints: kein int %s, %s", s, err.what()));
        }
    }
    return ints;
}

vector<long> read_longs(const std::string& filename) {
    vector<long> longs;
    for (const string& lword :
             read_accept_file(filename, CHARS_INTEGRAL_WORDS)) {
        try {
            const long val = stol(lword);
            longs.push_back(val);
        } catch (const invalid_argument& err) {
            auto s = lword.c_str();
            throw error(format("read_longs: kein long %s, %s", s, err.what()));
        }
    }
    return longs;
}

static const char CHARS_FLOAT_WORDS[] = "-+0123456789.";
vector<double> read_doubles(const std::string& filename) {
    vector<double> doubles;
    for (const string& dword :
             read_accept_file(filename, CHARS_FLOAT_WORDS)) {
        try {
            const double val = stod(dword);
            doubles.push_back(val);
        } catch (const invalid_argument& err) {
            auto s = dword.c_str();
            auto e = err.what();
            throw error(format("read_doubles: kein double %s, %s", s, e));
        }
    }
    return doubles;
}

vector<vector<int>> read_2ints(const std::string& filename) {
    vector<vector<int>> intss;
    for (const string& line : read_lines(filename)) {
        vector<int> row;
        for (const string& iword :
                 read_accept_str(line, CHARS_INTEGRAL_WORDS)) {
            try {
                const int val = stoi(iword);
                row.push_back(val);
            } catch (const invalid_argument& err) {
                auto s = iword.c_str();
                auto e = err.what();
                throw error(format("read_2ints: kein int %s, %s", s, e));
            }
        }
        intss.push_back(row);
    }
    return intss;
}

vector<vector<double>> read_2doubles(const std::string& filename) {
    vector<vector<double>> doubless;
    for (const string& line : read_lines(filename)) {
        vector<double> row;
        for (const string& dword :
                 read_accept_str(line, CHARS_FLOAT_WORDS)) {
            try {
                const double val = stod(dword);
                row.push_back(val);
            } catch (const invalid_argument& err) {
                auto s = dword.c_str();
                auto e = err.what();
                throw error(format("read_2doubles: kein double %s, %s", s, e));
            }
        }
        doubless.push_back(row);
    }
    return doubless;
}

static bool str2int(const string& word, int& val) {
    if (word.size() == 0) {
        return false;
    }
    if (word.find_first_not_of(CHARS_INTEGRAL_WORDS) != string::npos) {
        return false;
    }
    // does not capture +/- within digits
    val = stoi(word);
    return true;
}

// eat whitespace and all following comment lines
static int skip_comments(const vector<char>& raw, int cur) {
    const int size = static_cast<int>(raw.size()); // int is sufficient
    if (cur < 0 || cur >= size) {
        return cur;
    }
    while (cur < size && isspace(raw[static_cast<size_t>(cur)])) {
        cur += 1;
    }
    if (cur >= size) {
        return cur;
    }
    if (cur > 0) {
        // cur - 1 is >= 0
        if (raw[static_cast<size_t>(cur - 1)] != '\n') { // must be a new line
            return cur;
        }
    }
    if (raw[static_cast<size_t>(cur)] == '#') { // comment start in new line
        while (cur < size && raw[static_cast<size_t>(cur)] != '\n') {
            cur += 1;
        } // run until end of line
        if (cur < size) {
            cur += 1; // skip newline
        }
        return skip_comments(raw, cur); // again
    }
    return cur;
}

// reads a number in a char array and positions cur after the last digit
// returns cur, modifies parameter val
static int read_asciiint(const vector<char>& raw, int cur, int& val) {
    val = 0;
    if (cur < 0) { // already in error state
        return cur;
    }
    cur = skip_comments(raw, cur);
    const int size = static_cast<int>(raw.size()); // int is sufficient
    while (cur < size && isspace(raw[static_cast<size_t>(cur)])) {
        cur += 1;
    }
    if (cur == size) {
        return -1;
    }
    bool valid = false;
    while (cur < size && isdigit(raw[static_cast<size_t>(cur)])) {
        valid = true;
        val *= 10;
        // digit to value, guaranteed by the standard
        val += raw[static_cast<size_t>(cur)] - '0'; 
        cur += 1;
    }
    return valid ? cur : -1;
}

// binary pgm, P5
static std::vector<int> read_pgm5(const string& filename) {
    vector<char> raw = read_bytes(filename); // binary format
    if (raw[0] != 'P' || raw[1] != '5') {
        throw error("read_pgm::kein P5???");
    }
    int cur = 2; // we are here
    int width, height, maxbright;
    cur = read_asciiint(raw, cur, width);
    cur = read_asciiint(raw, cur, height);
    cur = read_asciiint(raw, cur, maxbright);
    if (cur == -1) {
        auto s = filename.c_str();
        auto e = "keine Breite/Hoehe/Helligkeit";
        throw error(format("read_pgm5:: filename=%s, %s", s, e));
    }
    // a single whitespace, often '\n'
    // can be windows? thus two chars?
    if (raw[static_cast<size_t>(cur)] == '\r') { 
        cur += 1;
        if (cur == -1) {
            throw error(format("read_pgm5:: filename=%s, Windows and end?",
                               filename.c_str()));
        }
    }
    if (!isspace(raw[static_cast<size_t>(cur)])) {
        throw error(format("read_pgm5:: filename=%s, kein Weissraum nach spec",
                           filename.c_str()));
    }
    cur += 1;
    // ab hier binary
    const bool twobyte = maxbright >= 256;
    const size_t offset = static_cast<size_t>(cur);
    const size_t len_bytes = raw.size() - offset;
    const size_t needed = static_cast<size_t>((twobyte ? 2 : 1) * width * height);
    if (len_bytes != needed) {
        string fmt = "filename=%s, width=%d, height=%d, maxbright=%d,\n";
        fmt = "read_pgm5:: " + fmt;
        fmt += "           len_bytes=%d, needed=%d";
        auto s = filename.c_str();
        throw error(format(fmt, s, width, height, maxbright, len_bytes,
                           needed));
    }
    vector<int> img(needed + 3);
    img[0] = width;
    img[1] = height;
    img[2] = maxbright;
    if (twobyte) {
        for (size_t i = 0; i < needed; i += 1) {
            char high_ch = raw[offset + 2 * i];
            img[i + 3] = reinterpret_cast<unsigned char&>(high_ch) * 256;
            char low_ch = raw[offset + 2 * i + 1];
            img[i + 3] += reinterpret_cast<unsigned char&>(low_ch);
        }
    } else {
        for (size_t i = 0; i < needed; i += 1) {
            char ch = raw[offset + i];
            img[i + 3] = reinterpret_cast<unsigned char&>(ch);
        }
    }
    return img;
}

// ascii/plain pgm, P2
static std::vector<int> read_pgm2(const string& filename) {
    vector<string> raw_lines = read_lines(filename);
    vector<string> lines;
    // no STL remove_if, as the compile time explodes
    for (const string& line : raw_lines) {
        if (!starts_with(line, "#")) { // ignore comments
            // only nonempty line, empty lines ignored
            if (line.find_first_of(CHARS_GERMAN_WORDS) != string::npos) {
                lines.push_back(line);
            }
        }
    }
    vector<string> words;
    auto lit = lines.begin();
    while (lit != lines.end() && words.size() < 4) {
        if (words.size() == 0) {
            read_accept_str_vec(*lit, CHARS_GERMAN_WORDS, words); // magic
        } else {
            read_accept_str_vec(*lit, CHARS_INTEGRAL_WORDS, words);
        }
        ++lit;
    }
    if (words.size() < 4) {
        throw pbma_exception("keine PGM-Datei, zu kurz: ", filename);
    }
    // we have enough to identify the file
    if (words[0] != "P2") { // we use plain format
        // for(auto word : words) { cout << word << " "; }; cout << endl;
        throw pbma_exception("keine PGM-Datei, kein P2-magic: ", filename);
    }
    int width, height, maxbright;
    if (!str2int(words[1], width)) {
        throw pbma_exception("keine PGM-Datei, Breite fehlt: ", filename);
    }
    if (!str2int(words[2], height)) {
        throw pbma_exception("keine PGM-Datei, Hoehe fehlt: ", filename);
    }
    if (!str2int(words[3], maxbright)) {
        throw pbma_exception("keine PGM-Datei, keine maximale Helligkeit",
                             filename);
    }
    // cout << "w=" << width << ", h=" << height;
    // cout << ", mb=" << maxbright << endl;
    vector<int> img;
    img.reserve(static_cast<size_t>(width * height + 3));
    img.push_back(width);
    img.push_back(height);
    img.push_back(maxbright);
    words.erase(words.begin(), words.begin() + 4);
    while (lit != lines.end()) {
        read_accept_str_vec(*lit, CHARS_INTEGRAL_WORDS, words);
        for (auto word : words) {
            int val;
            if (!str2int(word, val)) {
                throw pbma_exception("Keine PGM-Datei, Pixel: ", word);
            }
            img.push_back(val);
        }
        words.clear();
        ++lit;
    }
    if (img.size() != static_cast<unsigned int>(width * height + 3)) {
        throw pbma_exception("Keine PGM-Datei, falsche Anzahl Pixel: ",
                             static_cast<int>(img.size()));
    }
    return img;
}

std::vector<int> read_pgm(const std::string& filename) {
    ifstream is(filename, ifstream::binary);
    if (!is) {
        auto s = filename.c_str();
        throw error(format("read_pgm: filename=%s nicht lesbar", s));
    }
    // check magic number (P2 or P5)
    vector<char> buffer(2); // identifier has two chars
    is.read(buffer.data(), 2);
    if (!is.good()) {
        auto s = filename.c_str();
        throw error(format("read_pgm: filename=%s Einlesefehler", s));
    }
    is.close();
    if (buffer[0] != 'P') {
        auto s = filename.c_str();
        string msg = "filename=%s magic must start with 'P' not '%c'";
        msg = "read_pgm: " + msg;
        throw error(format(msg, s, buffer[0]));
    }
    // based on magic number chose right reader
    switch (buffer[1]) {
    case '5':
        return read_pgm5(filename);
        // no fall through as there is a return
    case '2':
        return read_pgm2(filename);
        // no fall through as there is a return
    default: {
        auto s = filename.c_str();
        string msg = "filename=%s magic '2' or '5' not '%c'";
        msg = "read_pgm: " + msg;
        throw error(format(msg, s, buffer[1]));
    }
    }
}

static bool is_pgm(const vector<int>& img) {
    const size_t imgsize = img.size();
    if (imgsize < 3) {
        cerr << "is_pgm: nicht gross genug" << endl;
        return false;
    }
    if (imgsize != static_cast<size_t>(img[0] * img[1] + 3)) {
        cerr << "is_pgm: falsche Dimension: ";
        cerr << img[0] << "x" << img[1] << endl;
        return false;
    }
    const int maxbright = img[2];
    for (size_t idx = 3; idx < imgsize; idx += 1) {
        if (img[idx] > maxbright) {
            cerr << "is_pgm: Pixel zu hell: @" << idx << endl;
            return false;
        }
        if (img[idx] < 0) {
            cerr << "is_pgm: Pixel zu dunkel: @" << idx << endl;
            return false;
        }
    }
    return true;
}

// ascii/plain pgm, P2
static void save_pgm2(const string& filename, const vector<int>& img) {
    ofstream out(filename);
    if (!out.is_open()) {
        throw pbma_exception("save_pgm, kann nicht schreiben: ", filename);
    }
    out << "P2\n";
    // dimension
    out << img[0] << " " << img[1] << "\n";
    // max brightness
    out << img[2] << "\n";
    // with 10 per line we are below 70 chars if max digits is 6
    int count = 0;
    for (auto it = img.begin() + 3; it != img.end(); ++it) {
        out << *it;
        count += 1;
        if (count % 10 == 0) {
            out << "\n";
        } else {
            out << " ";
        }
    }
    out.close();
}

// binary pgm, P5
static void save_pgm5(const string& filename, const vector<int>& img) {
    ofstream out(filename, ofstream::binary);
    if (!out.is_open()) {
        throw pbma_exception("save_pgm, kann nicht schreiben: ", filename);
    }
    out << "P5\n";
    // dimension
    out << img[0] << " " << img[1] << "\n";
    // max brightness
    out << img[2];
    // single whitespace
    out << '\n';
    if (img[2] < 256) {
        for (auto it = img.begin() + 3; it != img.end(); ++it) {
            const unsigned char b = static_cast<unsigned char>(*it);
            out << b;
        }
    } else { // two byte
        for (auto it = img.begin() + 3; it != img.end(); ++it) {
            const unsigned char b1 = static_cast<unsigned char>(*it >> 8);
            const unsigned char b2 = static_cast<unsigned char>(*it & 0xff);
            out << b1; // msb first
            out << b2;
        }
    }
    out.close();
}

void save_pgm(const string& filename, const vector<int>& img, bool plain) {
    if (!is_pgm(img)) {
        throw pbma_exception("save_pgm, keine PGM-Datei: ", filename);
    }
    if (plain) {
        save_pgm2(filename, img);
    } else {
        save_pgm5(filename, img);
    }
}

static bool contains(const vector<string>& svec, const string& ele) {
    return any_of(begin(svec), end(svec),
                  [&](const std::string& s) noexcept { return s == ele; });
}

// removes up to two leading dashes from a string
static inline string remove_dashes(string s) {
    if (starts_with(s, "--")) {
        return s.substr(2);
    } else if (starts_with(s, "-")) {
        return s.substr(1);
    }
    return s;
}

args_t::args_t(int argc, char* argv[]) {
    init(argc, argv);
}

args_t::args_t(int argc, const char* argv[]) {
    // oh, just make it work, we do not write anyway
    init(argc, const_cast<char**>(argv));
}

// the args arguments parser
void args_t::init(int argc, char* argv[]) {
    _program = argv[0];
    for (int i = 1; i < argc; i += 1) {
        string arg{argv[i]};
        // flag or option and not a negative number
        if (starts_with(arg, "-") and
            (arg.size() >= 2 and (arg[1] < '0' or arg[1] > '9'))) {
            arg = remove_dashes(arg);
            const auto it = arg.find("=");
            if (it != string::npos) { // with equals sign -> option
                string key = arg.substr(0, it);
                string val = arg.substr(it + 1);
                _options[key] = val;
            } else { // -> flag
                if (!contains(_flags, arg)) {
                    _flags.push_back(arg);
                }
            }
        } else { // positional
            _positionals.push_back(arg);
        }
    }
}

string args_t::program() const {
    return _program;
}

bool args_t::flag(const string& key) const {
    return contains(_flags, key);
}

size_t args_t::len_flags() const {
    return _flags.size();
}

vector<string> args_t::flags() const {
    vector<string> ret = _flags; // a copy
    // move semantics prevent second copy
    return ret;
}

bool args_t::has_option(const string& key) const {
    return _options.find(key) != _options.end();
}

size_t args_t::len_options() const {
    return _options.size();
}

vector<string> args_t::options() const {
    vector<string> ret;
    transform(begin(_options), end(_options),
              back_inserter(ret),
              [](const pair<string, string>& p) {
                  return p.first;
              });
    return ret;
}

std::string args_t::option(const std::string& key) const {
    auto it = _options.find(key);
    if (it == _options.end()) {
        throw error(format("args::option: no key %s", key.c_str()));
    }
    return it->second;
}

std::string args_t::option(const std::string& key,
                           const std::string& defval) const {
    auto it = _options.find(key);
    if (it == _options.end()) {
        return defval;
    }
    return it->second;
}

int args_t::int_option(const std::string& key) const {
    auto it = _options.find(key);
    if (it == _options.end()) {
        throw error(format("args::int_option: no %s", key.c_str()));
    }
    try {
        size_t idx;
        int val = stoi(it->second, &idx);
        if (it->second[idx] != 0) {
            throw invalid_argument("no just an int");
        }
        return val;
    } catch (const invalid_argument&) { // exception not used
        auto k = key.c_str();
        auto s = it->second.c_str();
        throw error(format("args::int_option(%s): no int %s", k, s));
    }
}

int args_t::int_option(const std::string& key, int defval) const {
    auto it = _options.find(key);
    if (it == _options.end()) {
        return defval;
    }
    try {
        size_t idx;
        int val = stoi(it->second, &idx);
        if (it->second[idx] != 0) {
            throw invalid_argument("no just an int");
        }
        return val;
    } catch (const invalid_argument&) { // exception not used
        auto msg = "args::int_option(%s, %d): no int %s";
        auto k = key.c_str();
        auto s = it->second.c_str();
        throw error(format(msg, k, defval, s));
    }
}

long args_t::long_option(const std::string& key) const {
    auto it = _options.find(key);
    if (it == _options.end()) {
        throw error(format("args::long_option: no %s", key.c_str()));
    }
    try {
        size_t idx;
        long val = stol(it->second, &idx);
        if (it->second[idx] != 0) {
            throw invalid_argument("no just a long");
        }
        return val;
    } catch (const invalid_argument&) { // exception not used
        auto k = key.c_str();
        auto s = it->second.c_str();
        throw error(format("args::long_option(%s): no long %s", k, s));
    }
}

long args_t::long_option(const std::string& key, long defval) const {
    auto it = _options.find(key);
    if (it == _options.end()) {
        return defval;
    }
    try {
        size_t idx;
        long val = stol(it->second, &idx);
        if (it->second[idx] != 0) {
            throw invalid_argument("no just a long");
        }
        return val;
    } catch (const invalid_argument&) { // exception not used
        auto k = key.c_str();
        auto s = it->second.c_str();
        throw error(format("args::long_option(%s): no long %s", k, s));
    }
}

double args_t::double_option(const std::string& key) const {
    auto it = _options.find(key);
    if (it == _options.end()) {
        throw error(format("args::double_option: no %s", key.c_str()));
    }
    try {
        size_t idx;
        double val = stod(it->second, &idx);
        if (it->second[idx] != 0) {
            throw invalid_argument("no just a double");
        }
        return val;
    } catch (const invalid_argument&) { // exception not used
        auto k = key.c_str();
        auto s = it->second.c_str();
        throw error(format("args::double_option(%s): no double %s", k, s));
    }
}

double args_t::double_option(const std::string& key, double defval) const {
    auto it = _options.find(key);
    if (it == _options.end()) {
        return defval;
    }
    try {
        size_t idx;
        double val = stod(it->second, &idx);
        if (it->second[idx] != 0) {
            throw invalid_argument("no just a double");
        }
        return val;
    } catch (const invalid_argument&) { // exception not used
        auto k = key.c_str();
        auto s = it->second.c_str();
        throw error(format("args::double_option(%s): no double %s", k, s));
    }
}

vector<string> args_t::positionals() const {
    return _positionals; // a copy on purpose
}

size_t args_t::len_pos() const noexcept {
    // we may cache, but efficiency is not important here
    return _positionals.size();
}

string args_t::pos(size_t idx) const {
    if (idx >= _positionals.size()) {
        throw error(format("args::pos: no idx %zu", idx));
    }
    return _positionals[idx];
}

string args_t::pos(size_t idx, const std::string& defval) const {
    if (idx >= _positionals.size()) {
        return defval;
    }
    return _positionals[idx];
}

vector<int> args_t::int_positionals() const {
    vector<int> ret;
    for (size_t idx = 0; idx < _positionals.size(); idx += 1) {
        ret.push_back(int_pos(idx));
    }
    return ret;
}

int args_t::int_pos(size_t idx) const {
    if (idx >= _positionals.size()) {
        throw error(format("args::int_pos: no idx %zu", idx));
    }
    try {
        size_t lidx;
        const int val = stoi(_positionals[idx], &lidx);
        if (_positionals[idx][lidx] != 0) {
            throw invalid_argument("no just an int");
        }
        return val;
    } catch (const invalid_argument&) { // exception not used
        auto s = _positionals[idx].c_str();
        throw error(format("args::int_pos(%d): no int %s", idx, s));
    }
}

int args_t::int_pos(size_t idx, int defval) const {
    if (idx >= _positionals.size()) {
        return defval;
    }
    try {
        size_t lidx;
        const int val = stoi(_positionals[idx], &lidx);
        if (_positionals[idx][lidx] != 0) {
            throw invalid_argument("no just an int");
        }
        return val;
    } catch (const invalid_argument&) { // exception not used
        auto s = _positionals[idx].c_str();
        throw error(format("args::int_pos(%zu): no int %s", idx, s));
    }
}

vector<long> args_t::long_positionals() const {
    vector<long> ret;
    for (size_t idx = 0; idx < _positionals.size(); idx += 1) {
        ret.push_back(long_pos(idx));
    }
    return ret;
}

long args_t::long_pos(size_t idx) const {
    if (idx >= _positionals.size()) {
        throw error(format("args::long_pos: no idx %zu", idx));
    }
    try {
        size_t lidx;
        const long val = stol(_positionals[idx], &lidx);
        if (_positionals[idx][lidx] != 0) {
            throw invalid_argument("no just a long");
        }
        return val;
    } catch (const invalid_argument&) { // exception not used
        auto s = _positionals[idx].c_str();
        throw error(format("args::long_pos(%zu): no long %s", idx, s));
    }
}

long args_t::long_pos(size_t idx, long defval) const {
    if (idx >= _positionals.size()) {
        return defval;
    }
    try {
        size_t lidx;
        const long val = stol(_positionals[idx], &lidx);
        if (_positionals[idx][lidx] != 0) {
            throw invalid_argument("no just a long");
        }
        return val;
    } catch (const invalid_argument&) { // exception not used
        auto s = _positionals[idx].c_str();
        throw error(format("args::long_pos(%zu): no long %s", idx, s));
    }
}

vector<double> args_t::double_positionals() const {
    vector<double> ret;
    for (size_t idx = 0; idx < _positionals.size(); idx += 1) {
        ret.push_back(double_pos(idx));
    }
    return ret;
}

double args_t::double_pos(size_t idx) const {
    if (idx >= _positionals.size()) {
        throw error(format("args::double_pos: no idx %zu", idx));
    }
    try {
        size_t lidx;
        const double val = stod(_positionals[idx], &lidx);
        if (_positionals[idx][lidx] != 0) {
            throw invalid_argument("no just a double");
        }
        return val;
    } catch (const invalid_argument&) { // exception not used
        auto s = _positionals[idx].c_str();
        throw error(format("args::double_pos(%zu): no double %s", idx, s));
    }
}

double args_t::double_pos(size_t idx, double defval) const {
    if (idx >= _positionals.size()) {
        return defval;
    }
    try {
        size_t lidx;
        const double val = stod(_positionals[idx], &lidx);
        if (_positionals[idx][lidx] != 0) {
            throw invalid_argument("no just a double");
        }
        return val;
    } catch (const invalid_argument&) { // exception not used
        auto s = _positionals[idx].c_str();
        throw error(format("args::double_pos(%zu): no double %s", idx, s));
    }
}

static vector<int> _create_randints(int how_many, int lower, int upper) {
    vector<int> ret(static_cast<size_t>(how_many));
    int modulo = (upper - lower) + 1;
    if (modulo < 0) {
        modulo = upper;
    }
    for (size_t i = 0; i < static_cast<size_t>(how_many); i += 1) {
        ret[i] = (static_cast<int>(rand()) % modulo) + lower;
    }
    return ret;
}

vector<int> create_randints(int how_many, int lower, int upper) {
    const chrono::high_resolution_clock::time_point beginning =
        chrono::high_resolution_clock::now();
    auto seed = beginning.time_since_epoch().count() % 2097152;
    srand(static_cast<unsigned int>(seed));
    return _create_randints(how_many, lower, upper);
}

vector<int> create_same_randints(int how_many, int lower, int upper) {
    srand(1234567); // default seed fixed number
    return _create_randints(how_many, lower, upper);
}

bool is_sorted(vector<int>& a, int& first_error) noexcept {
    const size_t asize = a.size();
    for (size_t i = 1; i < asize; i += 1) {
        if (a[i - 1] > a[i]) {
            first_error = static_cast<int>(i); // int is sufficient
            return false;
        }
    }
    first_error = 0;
    return true;
}

bool check_sort_one(sort_function sort, int size, bool timing, int verbose) {
    vector<int> a = create_randints(size);
    int first_error;
    if (is_sorted(a, first_error)) {
        if (verbose > 10) {
            cout << "Warnung, ursprüngliches Feld schon sortiert";
        }
    }
    long sum = accumulate(begin(a), end(a), 0); // sum up
    const Timer timer;
    reset_swaps();
    sort(a);
    const int swaps = get_swaps();
    string measure = timer.human_measure();
    if (!is_sorted(a, first_error)) {
        const auto fem1 = first_error - 1;
        cout << "Fehler: Feld a[" << a.size() << "] nicht sortiert, ";
        cout << "a[" << fem1 << "]=";
        cout << a[static_cast<size_t>(fem1)] << " > ";
        cout << "a[" << first_error << "]=";
        cout  << a[static_cast<size_t>(first_error)] << endl;
        return false;
    }
    sum -= accumulate(begin(a), end(a), 0); // sum down
    if (sum != 0) { // checksum failed
        cout << "Fehler: Feld a[" << a.size() << "] sortiert, ";
        cout << "aber andere Werte als im ursprünglichen Feld";
        return false;
    }
    if (verbose >= 1) {
        cout << "a[" << setw(8) << size << "]: sorted ";
        if (timing) {
            cout << " " << measure;
        }
        if (swaps > 0) { // assume it is used
            cout << " swaps=" << setw(10) << swaps;
        }
        cout << endl;
    }
    return true;
}

bool check_sort(sort_function sort, bool timing, bool large, int verbose) {
    vector<int> sort_sizes = {10, 100, 1000, 10000, 20000, 40000, 60000};
    for (const int size : sort_sizes) {
        if (!check_sort_one(sort, size, timing, verbose)) {
            return false;
        }
    }
    if (large) {
        vector<int> large_sort_sizes = {80000, 100000, 200000, 400000, 800000, 1000000};
        for (const int size : large_sort_sizes) {
            if (!check_sort_one(sort, size, timing, verbose)) {
                return false;
            }
        }
    }
    return true;
}

int _swaps_counter = 0; // not static, as inlined in header in swap

int get_swaps() noexcept {
    return _swaps_counter;
}

int reset_swaps() noexcept {
    const int ret = _swaps_counter;
    _swaps_counter = 0;
    return ret;
}

Timer::Timer() noexcept {
    restart();
}

double Timer::measure() const noexcept {
    const auto end = chrono::system_clock::now();
    const chrono::duration<double> elapsed = end - start;
    return elapsed.count();
}

long Timer::measure_ms() const noexcept{
    const auto end = chrono::system_clock::now();
    const auto elapsed = chrono::duration_cast<chrono::milliseconds>(end - start);
    return static_cast<long>(elapsed.count());
}

long Timer::measure_us() const noexcept {
    const auto end = chrono::system_clock::now();
    const auto elapsed = chrono::duration_cast<chrono::microseconds>(end - start);
    return static_cast<long>(elapsed.count());
}

long Timer::measure_ns() const noexcept {
    const auto end = chrono::system_clock::now();
    const auto elapsed = chrono::duration_cast<chrono::nanoseconds>(end - start);
    return static_cast<long>(elapsed.count());
}

void Timer::restart() noexcept {
    start = chrono::system_clock::now();
}

// note, that all three are full and not truncated (modulo)
static string _human_format(long nanos, long mikros, long millis) {
    // check for millis to prevent overflow on 32 bit systems
    if (millis == 0 && nanos <= 999) {
        return format("%3dns", nanos);
    }
    if (millis == 0 && nanos <= 9999) {
        return format("%1d.%1dus", nanos / 1000, (nanos / 100) % 10);
    }
    if (millis == 0 && mikros <= 999) {
        return format("%3dus", mikros);
    }
    if (millis >= 1 && mikros <= 9999) {
        return format("%1d.%1dms", mikros / 1000, (mikros / 100) % 10);
    }
    if (millis <= 999) {
        return format("%3dms", millis);
    }
    if (millis <= 99999) {
        return format("%2d.%1ds", millis / 1000, (millis / 100) % 10);
    }
    const long seconds = millis / 1000;
    return format("%ds", seconds);
}

string Timer::human_measure() const {
    const long nanos = measure_ns();
    const long mikros = measure_us(); // needed for systems with 32 bit long
    const long millis = measure_ms(); // needed for systems with 32 bit long
    return _human_format(nanos, mikros, millis);
}

string Timer::human_format(double secs) {
    const uint64_t ns = static_cast<uint64_t>(secs * 1000000000);
    const uint64_t us = ns / 1000;
    const uint64_t ms = us / 1000;
    return _human_format(static_cast<long>(ns), static_cast<long>(us),
                         static_cast<long>(ms));
}

void schlafe_ms(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void schlafe_us(int us) {
    std::this_thread::sleep_for(std::chrono::microseconds(us));
}

BigInt::BigInt(long long val) {
    digits = to_string(val); // note, that 0 has sign 1 here
    if (digits[0] == '-') {
        digits.erase(0, 1);
        sign = -1;
    } else {
        sign = 1;
    }
}

void BigInt::init(unsigned long long val) {
    digits = to_string(val);
    sign = 1;
}

static bool is_decimalstring(const string& digits) {
    if (digits.empty()) {
        return false;
    }
    auto it = digits.cbegin();
    if (*it == '+') {
        ++it;
    } else if (*it == '-') {
        ++it;
    }
    while (it != digits.cend()) {
        if (*it < '0' || *it > '9') {
            return false;
        }
        ++it;
    }
    return true;
}

BigInt::BigInt(const string& _digits) {
    if (!is_decimalstring(_digits)) {
        cerr << "BigInt(string): ungueltige Zahl, " << _digits << endl;
        throw pbma_exception("BigInt(string): ungueltige Zahl, ", _digits);
    }
    stringstream ss(_digits);
    ss >> *this;  
}

ostream& operator<<(ostream& out, const BigInt& bi) {
    if (bi.sign == -1) {
        out << ("-" + bi.digits); // to ensure that a preceding setw works
    } else {
        out << bi.digits;
    }
    return out;
}

istream& operator>>(istream& in, BigInt& bi) {
    string buffer;
    in >> buffer;
    if (buffer.empty()) {
        in.setstate(ios::failbit);
        return in;
    }
    int sign = 1;
    if (buffer[0] == '-') {
        sign = -1;
        buffer.erase(0, 1);
    } else if (buffer[0] == '+') {
        buffer.erase(0, 1);
    }
    // remove leading zeros
    bool didremove = false;
    while (!buffer.empty() && buffer[0] == '0') {
        buffer.erase(0, 1);
        didremove = true;
    }
    if (buffer.empty()) {
        if (didremove) {
            sign = 1;
            buffer = "0";
        } else {
            in.setstate(ios::failbit);
            return in;
        }
    }
    for (char ch : buffer) {
        if (ch < '0' || '9' < ch) {
            in.setstate(ios::failbit);
            return in;
        }
    }
    bi.sign = sign;
    bi.digits = buffer;
    return in;
}

string to_string(const BigInt& bi) {
    if (bi.sign == -1) {
        return "-" + bi.digits;
    } else {
        return bi.digits;
    }
}

// we do not cover the smallest possible long
long BigInt::to_long() const {
  long val = 0;
  for (auto ch : digits) {
    if (val > LONG_MAX/10) {
        throw pbma_exception("BigInt::to_long: too long");
    }    
    val *= 10;
    if (val > LONG_MAX - (ch - '0')) {
        throw pbma_exception("BigInt::to_long: too long");
    }
    val += (ch - '0');
  }
  return sign*val;
}

// we do not cover the smallest possible long long
long long BigInt::to_long_long() const {
    long long val = 0;
    for (auto ch : digits) {
        if (val > LLONG_MAX/10) {
            throw pbma_exception("BigInt::to_long_long: too long");
        }    
        val *= 10;
        if (val > LLONG_MAX - (ch - '0')) {
            throw pbma_exception("BigInt::to_long_long: too long");
        }
        val += (ch - '0');
    }
    return sign*val;
}

// compare two digit strings without sign
static int compare_digitstrings(const string& d1, const string& d2) {
    if (d1.size() < d2.size()) {
        return -1;
    }
    if (d1.size() > d2.size()) {
        return 1;
    }
    // same size, lexicographic ordering
    if (d1 < d2) {
        return -1;
    } else if (d1 > d2) {
        return 1;
    } else { // == 
        return 0;
    }
}

// adds three literals
static inline pair<char, char> add(char a, char b, char c) {
    int val = (a - '0') + (b - '0') + (c - '0');
    return {val / 10 + '0', val % 10 + '0'};
}

// adds two digit strings
static inline string add(const string& d1, const string& d2) {
    vector<char> res;
    auto it1 = d1.crbegin();
    auto it1e = d1.crend();
    auto it2 = d2.crbegin();
    auto it2e = d2.crend();
    char carry = '0';
    while (it1 != it1e && it2 != it2e) {
        auto p = add(*it1, *it2, carry);
        carry = p.first;
        res.push_back(p.second);
        ++it1;
        ++it2;
    }
    while (it1 != it1e) {
        auto p = add(*it1, '0', carry);
        carry = p.first;
        res.push_back(p.second);
        ++it1;
    }
    while (it2 != it2e) {
        auto p = add('0', *it2, carry);
        carry = p.first;
        res.push_back(p.second);
        ++it2;
    }
    if (carry != '0') {
        res.push_back(carry);
    }
    return string(res.crbegin(), res.crend());
}

// subtracts two literals from first
static inline pair<char, char> subtract(char a, char b, char c) {
    int val = (a - '0') - ((b - '0') + (c - '0'));
    char carry = '0';
    if (val < 0) {
        carry = '1';
        val += 10;
    } 
    return {carry, val + '0'};
}

// subtracts two digit strings, where first one *must* be larger
static inline string subtract_fromlarger(const string& d1, 
                                         const string& d2) {
    vector<char> res;
    auto it1 = d1.crbegin();
    auto it1e = d1.crend();
    auto it2 = d2.crbegin();
    auto it2e = d2.crend();
    char carry = '0';
    while (it1 != it1e && it2 != it2e) {
        auto p = subtract(*it1, *it2, carry);
        carry = p.first;
        res.push_back(p.second);
        ++it1;
        ++it2;
    }
    while (it1 != it1e) {
        auto p = subtract(*it1, '0', carry);
        carry = p.first;
        res.push_back(p.second);
        ++it1;
    }
    while (it2 != it2e) {
        auto p = subtract('0', *it2, carry);
        carry = p.first;
        res.push_back(p.second);
        ++it2;
    }
    if (carry != '0') {
        throw std::runtime_error("subtract_fromlarger: not larger");
    }
    // no leading 0's in representation
    while (!res.empty() && *res.rbegin() == '0') {
        res.pop_back();
    }
    return string(res.crbegin(), res.crend());
}

static inline pair<int, string> subtract(const string& d1, 
                                         const string& d2) {
    int cmp = compare_digitstrings(d1, d2); 
    if (cmp < 0) {
        return {-1, subtract_fromlarger(d2, d1)};
    } else if (cmp > 0) {
        return {1, subtract_fromlarger(d1, d2)};
    } else { // cmp == 0
        return {1, "0"};
    }
}

BigInt& BigInt::operator+=(const BigInt& other) {
    if (sign == other.sign) {
        digits = add(digits, other.digits);
    } else {
        if (sign == -1) {
            auto p = subtract(other.digits, digits);
            sign = p.first;
            digits = p.second;
        } else { // other.signs == -1
            auto p = subtract(digits, other.digits);
            sign = p.first;
            digits = p.second;
        }
    }
    return *this;
}

BigInt& BigInt::operator-=(const BigInt& other) {
    if (sign == other.sign) {
        auto p = subtract(digits, other.digits);
        sign = sign * p.first;
        digits = p.second;
    } else {
        digits = add(digits, other.digits);
        // keep sign, regardless (x it out...)
    }  
    return *this;
}

static inline string mult(const string& ds, char digit, int shift) {
    vector<char> res;
    int val = digit - '0';
    int carry = 0;
    for (auto it = ds.crbegin(); it != ds.crend(); ++it) {
        int cur = *it - '0';
        int mul = (cur * val) + carry;
        carry = mul / 10;
        res.push_back(static_cast<char>((mul % 10) + '0'));
    }
    if (carry > 0) {
        if (carry > 9) {
            string msg = "BigInt: mult, carry too large???";
            throw runtime_error(msg + " internal error");
        }
        res.push_back(static_cast<char>(carry + '0'));
    }
    // 10er places
    vector<char> tener;
    for (int i = 0; i < shift; ++i) {
        tener.push_back('0');
    }
    return string(res.crbegin(), res.crend()) +
        string(tener.begin(), tener.end());
}

static inline string mult(const string& d1, const string& d2) {
    string res = "0";
    int d2size = static_cast<int>(d2.size());
    for (int i = 0; i < d2size; ++i) {
        char digit = d2[static_cast<size_t>(d2size-1-i)]; // start with last
        string mres = mult(d1, digit, i);
        res = add(res, mres);
    }
    return res;
}

BigInt& BigInt::operator*=(const BigInt& other) {
    if (digits == "0" || other.digits == "0") {
        digits = "0";
        sign = 1;
    } else {
        sign *= other.sign;
        if (compare_digitstrings(digits, other.digits) < 0) {
            // use larger at the back, is faster
            digits = mult(other.digits, digits);
        } else {
            digits = mult(digits, other.digits);
        }
    }
    return *this;
}

// we use classic long division
// example 12345 / 67
//          67        1
//          564
//          536       8
//           285
//           268      4
//            17 (remainder ignore)
// we precompute possible fits (67, 536, 268) and just get the index
// working (123, 564, 285) is a string

// is d1 < d2, we know that d1.size() == d2.size()
static inline bool does_fit(const string& d1, const string& d2) {
    if (d1.size() < d2.size()) {
        return false;
    }
    if (d1.size() != d2.size()) {
        string msg = "BigInt, does_fit: d1.size() != d2.size()";
        throw std::runtime_error(msg + " internal error");
    }
    for (size_t i=0; i < d1.size(); ++i) {
        if (d1[i] < d2[i]) {
            return true;
        } else if (d1[i] > d2[i]) {
            return false;
        }
        // continue if same
    }
    return true; // all same, fits
}

// compute index of predefined multiples which fit tight in
// the working number (digits), the index is the resulting digit
// or 0 if it does not fit yet and needs more digits
int fit_it(const vector<string>& multiples, const string& working) {
    if (multiples[1].size() > working.size()) {
        // won't fit, not enough digits
        return 0;
    }
    int fit = 9;
    while (fit > 0) {
        if (multiples[static_cast<size_t>(fit)].size() < working.size()) {
            return fit; // less digits always fit
        }
        if (multiples[static_cast<size_t>(fit)].size() == working.size()) {
            if (does_fit(multiples[static_cast<size_t>(fit)], working)) {
                return fit;
            }
        }
        // if the multiples has more digits, it wont fit
        --fit;
    }
    return fit;
}

// d1 is definitely larger than d2, result will be >= 1
static inline string divide(const string& d1, const string& d2, int sign) {
    vector<string> multiples; // a table of mutiples of d2
    for (int i = 0; i < 10; ++i) {
        multiples.push_back(mult(d2, static_cast<char>(i + '0'), 0));
    } // ex: if d2 == 12 then  multiples[3] == 36
    string working = "0";
    vector<char> res;
    for (char digit : d1) {
        if (working[0] == '0') { // no leading zeros but single 0
            working = digit;
        } else {
            working += digit;
        }
        int fit = fit_it(multiples, working);
        if (fit == 0) {
            if (!res.empty()) {
                if (res[0] != '0') { // but no leading zero
                    res.push_back('0'); // don't forget the zeros
                }
            }
            continue;
        } 
        auto p = subtract(working, multiples[static_cast<size_t>(fit)]);
        if (p.first != 1) { // cannot be negative
            string msg = "BigInt: divide, subtract must not be negative";
            throw runtime_error(msg + " internal error");
        }
        working = p.second;
        while (!working.empty() && working[0] == '0') {
            // no leading 0's
            working.erase(0, 1);
        }
        // we know, that fit > 0
        res.push_back(static_cast<char>(fit + '0'));
    }
    string sres = string(res.cbegin(), res.cend()); // not reverse, right order
    // the remainder is in working;
    if (sign == -1 && !working.empty() && working != "0") {
        // well, on negative numbers and having a remainder we are
        // absolute one too low, as we need to round down which on
        // the negative side is one up
        sres = add(sres, "1");
    }
    return sres;
}

BigInt& BigInt::operator/=(const BigInt& other) {
    if (other.digits == "0") {
        throw runtime_error("BigInt::/=: division by 0");    
    }
    if (digits == "0") {
        return *this;
    }
    sign *= other.sign;
    int res = compare_digitstrings(digits, other.digits);
    if (res < 0) {
        if (sign == -1) {
            // round down, which is negative one if negative
            digits = "1";
        } else {
            digits = "0";
        }
    } else {
        digits = divide(digits, other.digits, sign);
    }
    return *this;
}

BigInt& BigInt::operator%=(const BigInt& other) {
    if (other.digits == "0") {
        throw pbma_exception("BigInt::%=: division by 0");    
    }
    if (digits == "0") {
        return *this;
    }
    int res = compare_digitstrings(digits, other.digits);
    if (res == 0) {
        sign = 1;
        digits = "0";
        return *this;
    }
    if (res < 0) {
        if (sign != other.sign) {
            // need to change digits and sign
            sign = other.sign;
            digits = subtract_fromlarger(other.digits, digits);
        }
        return *this;
    }
    // res > 0
    string sdiv = divide(digits, other.digits, 1);
    string smul = mult(sdiv, other.digits);
    if (digits == smul) {
        sign = 1;
        digits = "0";
        return *this;
    }
    digits = subtract_fromlarger(digits, smul); // the remainder
    if (sign != other.sign) {
        // need to change digits and sign
        sign = other.sign;
        digits = subtract_fromlarger(other.digits, digits);
    }
    return *this;
}

int BigInt::compare(const BigInt& other) const {
    if (sign > other.sign) {
        return 1;
    }
    if (sign < other.sign) {
        return -1;
    }
    // same sign
    int cmp = compare_digitstrings(digits, other.digits);
    return cmp*sign;
}

bool BigInt::equals(const BigInt& other) const {
    return sign == other.sign && digits == other.digits;
}
