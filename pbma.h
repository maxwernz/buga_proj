#ifndef PBMA_H
#define PBMA_H

/** @file
 * @brief Unterstützung für Objektorientierte Programmierung (OOP) 
 * und Algorithmen und Datenstrukturen (ADS)
 * Hilfsroutinen und Hilfsklassen, um Routineaufgaben zu vereinfachen
 * und sich je nach Aufgabenstellung auf das Wesentliche zu konzentrieren.
 * Es gibt Hilfestellungen, um Dateien einzulesen, Zeiten zu messen,
 * Kommandozeilenparameter zu verarbeiten und Zufallszahlen zu erzeugen.
 * Spezifische Unterstützung gibt es, um Ihr Sortierverfahren zu testen
 * und zu evaluieren.
 * Verwendung: Beide Quell-Dateien pbma.(h|cpp) in das Projekt kopieren.
 */

#include <chrono>
#include <exception>
#include <iomanip>
#include <iostream>
#include <map>
#include <string>
#include <vector>

/** Ausnahme, wenn etwas schief geht, wird diese Ausnahme von
 * den Funktionen geworfen.
 */
struct pbma_exception : public std::exception {
    std::string cause;
    explicit pbma_exception(const std::string& cause);
    pbma_exception(const std::string& cause, int val);
    pbma_exception(const std::string& cause, long val);
    pbma_exception(const std::string& cause, const std::string& val);
    const char* what() const noexcept override;
};

/** Beginnt ein String mit einem Präfix?
 * @param s zu durchsuchender String
 * @param prefix der Präfix
 * @return wahr gdw prefix ist Präfix von s
 */
bool starts_with(const std::string& s, const std::string& prefix);

/** Formatiert ein long mit minimaler Anzahl an Zeichen
 * @param val zu formatierender Wert
 * @param length Anzahl der Zeichen
 * @param fill_char mit welchem Zeichen wird aufgefüllt
 * @return formatierter String
 */
std::string format(long val, int length = 0, char fill_char = ' ');

/** Formatiert ein int mit minimaler Anzahl an Zeichen
 * @param val zu formatierender Wert
 * @param length Anzahl der Zeichen
 * @param fill_char mit welchem Zeichen wird aufgefüllt
 * @return formatierter String
 */
inline std::string format(int val, int length = 0, char fill_char = ' ') {
    return format(static_cast<long>(val), length, fill_char);
}

/** Prüft, ob Datei existiert durch Öffnen der Datei
 * @param filename Dateiname der zu prüfenden Datei
 * @return wahr gdw Datei war lesbar
 */
bool file_exists(const std::string& filename);

/** Vollständiges Einlesen einer Datei als vector<char>
 * @param filename Dateiname der einzulesenden Datei
 * @return binärer Inhalt der Datei
 */
std::vector<char> read_bytes(const std::string& filename);

/** Vollständiges Lesen einer Textdatei als vector<string>,
 * Kommentarzeilen (#..) ignorieren, leere Zeilen ignorieren,
 * Zeilen mit Whitespaces sind nicht leer
 * @param filename Dateiname der einzulesenden Datei
 * @return Zeilen der Datei
 */
std::vector<std::string> read_lines(const std::string& filename);

/** Wortweises Lesen einer Textdatei als vector<string>,
 * alphanumerische deutsche Worte, Satzzeichen etc. entfernen,
 * Kommentarzeilen (#..) ignorieren
 * @param filename Dateiname der einzulesenden Datei
 * @return Wörter der Datei
 */
std::vector<std::string> read_words(const std::string& filename);

/** Einlesen von Zahlen aus einer Textdatei als vector<int>,
 * numerische Wörter zu int konvertiert, Satzzeichen entfernen
 * Kommentarzeilen (#..) ignorieren, leere Zeilen ignorieren
 * @param filename Dateiname der einzulesenden Datei
 * @return ints der Datei
 */
std::vector<int> read_ints(const std::string& filename);

/** Einlesen von Zahlen aus einer Textdatei als vector<long>,
 * numerische Wörter zu long konvertiert, Satzzeichen entfernen
 * Kommentarzeilen (#..) ignorieren, leere Zeilen ignorieren
 * @param filename Dateiname der einzulesenden Datei
 * @return longs der Datei
 */
std::vector<long> read_longs(const std::string& filename);

/** Einlesen von Zahlen aus einer Textdatei als vector<double>,
 * numerische Wörter zu double konvertiert, Satzzeichen entfernen
 * Kommentarzeilen (#..) ignorieren, leere Zeilen ignorieren
 * @param filename Dateiname der einzulesenden Datei
 * @return doubles der Datei
 */
std::vector<double> read_doubles(const std::string& filename);

/** Einlesen von Zahlen aus einer Textdatei als
 * zweidimensionalen Vektor vector<vector<int>>,
 * Je Zeile steht ein Feld, muss nicht rechteckig (gleiche Dimensionen) sein
 * numerische Wörter zu int konvertiert, Satzzeichen entfernen
 * Kommentarzeilen (#..) ignorieren, leere Zeilen ignorieren
 * @param filename Dateiname der einzulesenden Datei
 * @return vector von vector von ints, zeilenweise der Datei
 */
std::vector<std::vector<int>> read_2ints(const std::string& filename);

/** Einlesen von Zahlen aus einer Textdatei als
 * zweidimensionalen Vektor vector<vector<double>>,
 * Je Zeile steht ein Feld, muss nicht rechteckig (gleiche Dimensionen) sein
 * numerische Wörter zu int konvertiert, Satzzeichen entfernen
 * Kommentarzeilen (#..) ignorieren, leere Zeilen ignorieren
 * @param filename Dateiname der einzulesenden Datei
 * @return vector von vector von doubles, zeilenweise der Datei
 */
std::vector<std::vector<double>> read_2doubles(const std::string& filename);

/** Einlesen eines Bilds im PGM-Format ASCII/plain (P2) und binär (P5).
 * Die ersten drei Zahlen sind Breite, Höhe, maximale Helligkeit.
 * Die folgenden Zahlen sind durch Whitespace separiert die Helligkeitswerte
 * der Pixel bei ASCII/plain oder nach der maximalen Helligkeit genau
 * ein Whitespace gefolgt von 1 Byte oder 2 Byte (msb) Graustufenwert.
 * http://netpbm.sourceforge.net/doc/pgm.html
 * @param filename Dateiname der einzulesenden PGM-Datei
 * @return img vector 3 Meta-Daten gefolgt von den Pixeln
 */
std::vector<int> read_pgm(const std::string& filename);

/** Abspeichern eines Bilds im (ASCII) PGM-Format
 * Die ersten drei Zahlen sind Breite, Höhe, maximale Helligkeit.
 * Die folgenden Zahlen sind zeilenweise die Helligkeitswerte der Pixel
 * http://netpbm.sourceforge.net/doc/pgm.html
 * @param filename Dateiname der PGM-Datei in die gespeichert wird
 * @param img vector 3 Meta-Daten gefolgt von den Pixeln
 * @param plain flag, ob ASCII/plain (P2) oder binär (P5), default P2
 * @return img vector 3 Meta-Daten gefolgt von den Pixeln
 */
void save_pgm(const std::string& filename, const std::vector<int>& img,
              bool plain = true);

/** Parsen von Kommandozeilenparameter
 * - Flags, ja/nein, Setzen mit führendem -(oder --)
 * - Optionen als Schlüssel/Wert-Paare -key=value oder --key=value
 * - Alles andere sind Positionswerte
 * nicht mehr.
 * Flags mit bool flag(string key) zum Testen auf ein Flag key
 * Optionen mit string option(string key) oder
   option(string key, string default_value):
 * - ein default Wert kann angegeben werden
 * - ein numerischer Typ kann erzwungen werden (int, long, double)
 * Alles was nicht Flag oder Option ist, ist ein Positionswert, vector.
 * Man sollte nicht Positionsparameter und andere mischen.
 */
class args_t {
private:
    std::string _program;
    std::vector<std::string> _positionals;
    std::vector<std::string> _flags;
    std::map<std::string, std::string> _options;

public:
    /** Konstruktor mit Argumenten von main, muss const... int main sein
     * @param argc Anzahl der Argumente
     * @param argv konstantes Feld von Char-Zeigern
     */
    args_t(int argc, const char* argv[]);
    /** Konstruktor mit Argumenten von main, wie im normalen Gebrauch
     * @param argc Anzahl der Argumente
     * @param argv Feld von Char-Zeigern
     */
    args_t(int argc, char* argv[]);

    /** Der Programmname, argv[0]
     * @return Programmname wie es aufgerufen wurde, argv[0]
     */
    std::string program() const;

    /** Check, ob flag key gesetzt ist
     * @param key zu testender Schlüssel
     * @return true gdw flag key gesetzt ist
     */
    bool flag(const std::string& key) const;

    /** Wie viele Flags (mit - oder -- beginnend) sind gesetzt
     * @return Anzahl der gesetzten Flags
     */
    size_t len_flags() const;

    /** Menge der (mit - oder -- beginnend) gesetzten Flags
     * @return Menge der gesetzten Flags
     */
    std::vector<std::string> flags() const;

    /** Gab es eine Option key=value (beinhaltet =) in den Argumenten
     * @param key gesetzter Schlüssel
     * @return true gdw key=value wurde gesetzt
     */
    bool has_option(const std::string& key) const;

    /** Anzahl der Optionen key=value (beinhaltet =) in den Argumenten
     * @return Anzahl der Optionen
     */
    size_t len_options() const;

    /** Menge aller Schlüssel aller gesetzten Optionen key=value
     * @return Menge der Schlüssel aller gesetzten Optionen
     */
    std::vector<std::string> options() const;

    /** Greife auf Option key=value zu, Ausnahme bei nicht gesetztem key
     * @param key gesetzter Schlüssel
     * @return value von Optione key=value
     */
    std::string option(const std::string& key) const;

    /** Greife auf Option key=value zu, Default-Wert bei nicht gesetztem key
     * @param key gesetzter Schlüssel
     * @param defval Vorgabewert falls Schlüssel nicht gesetzt ist
     * @return value von Optione key=value
     */
    std::string option(const std::string& key, const std::string& defval) const;

    /** Greife auf Integer-Option key=value zu, value konvertiert zu int
     * Ausnahme bei nicht gesetztem key
     * @param key gesetzter Schlüssel
     * @return value von Option key=value zu int konvertiert
     */
    int int_option(const std::string& key) const;

    /** Greife auf Integer-Option key=value zu, value konvertiert zu int
     * Default-Wert bei nicht gesetztem key
     * @param key gesetzter Schlüssel
     * @param defval Vorgabewert falls Schlüssel nicht gesetzt ist
     * @return value von Option key=value zu int konvertiert
     */
    int int_option(const std::string& key, int defval) const;

    /** Greife auf Long-Option key=value zu, value konvertiert zu long
     * Ausnahme bei nicht gesetztem key
     * @param key gesetzter Schlüssel
     * @return value von Option key=value zu long konvertiert
     */
    long long_option(const std::string& key) const;

    /** Greife auf Long-Option key=value zu, value konvertiert zu long
     * Default-Wert bei nicht gesetztem key
     * @param key gesetzter Schlüssel
     * @param defval Vorgabewert falls Schlüssel nicht gesetzt ist
     * @return value von Option key=value zu long konvertiert
     */
    long long_option(const std::string& key, long defval) const;

    /** Greife auf Double-Option key=value zu, value konvertiert zu double
     * Default-Wert bei nicht gesetztem key
     * @param key gesetzter Schlüssel
     * @return value von Option key=value zu double konvertiert
     */
    double double_option(const std::string& key) const;

    /** Greife auf Double-Option key=value zu, value konvertiert zu double
     * Ausnahme bei nicht gesetztem key
     * @param key gesetzter Schlüssel
     * @param defval Vorgabewert falls Schlüssel nicht gesetzt ist
     * @return value von Option key=value zu double konvertiert
     */
    double double_option(const std::string& key, double defval) const;

    /** Alle Argumente außer Programmname, Optionen und Flags (positionals)
     * @return Menge der Argumente, die weder
     *         Programmname, Option oder Flag sind
     */
    std::vector<std::string> positionals() const;

    /** Anzahl der Positionsparameter (positionals().size())
     * @return Anzahl der Positionsparameter
     */
    size_t len_pos() const noexcept;

    /** ites Element aller positionals (Nicht-Opts/Flags)
     * Ausnahme, falls ites Element nicht existiert
     * @param idx das wievielte Element, ites
     * @return iter Wert der Positionals als String
     */
    std::string pos(size_t idx) const;

    /** ites Element aller positionals (Nicht-Opts/Flags)
     * @param idx das wievielte Element, ites
     * @param defval Vorgabewert falls ites nicht existiert
     * @return iter Wert der Positionals als String oder defval
     */
    std::string pos(size_t idx, const std::string& defval) const;

    /** Alle Argumente als int
     * außer Programmname, Optionen und Flags (positionals)
     * @return Menge der Argumente als int, die weder
     *         Programmname, Option oder Flag sind
     */
    std::vector<int> int_positionals() const;

    /** ites Element aller positionals (Nicht-Opts/Flags) zu int konvertiert
     * Ausnahme, falls ites Element nicht existiert
     * @param idx das wievielte Element, ites
     * @return iter Wert der Positionals als int
     */
    int int_pos(size_t idx) const;

    /** ites Element aller positionals (Nicht-Opts/Flags) zu int konvertiert
     * @param idx das wievielte Element, ites
     * @param defval Vorgabewert falls ites nicht existiert
     * @return iter Wert der Positionals als int oder defval
     */
    int int_pos(size_t idx, int defval) const;

    /** Alle Argumente als long
     * außer Programmname, Optionen und Flags (positionals)
     * @return Menge der Argumente als long, die weder
     *         Programmname, Option oder Flag sind
     */
    std::vector<long> long_positionals() const;

    /** ites Element aller positionals (Nicht-Opts/Flags) zu long konvertiert
     * @param idx das wievielte Element, ites
     * @return iter Wert der Positionals als long
     */
    long long_pos(size_t idx) const;

    /** ites Element aller positionals (Nicht-Opts/Flags) zu long konvertiert
     * Ausnahme, falls ites Element nicht existiert
     * @param idx das wievielte Element, ites
     * @param defval Vorgabewert, falls ites nicht existiert
     * @return iter Wert der Positionals als long
     */
    long long_pos(size_t idx, long defval) const;

    /** Alle Argumente als double
     * außer Programmname, Optionen und Flags (positionals)
     * @return Menge der Argumente als double, die weder
     *         Programmname, Option oder Flag sind
     */
    std::vector<double> double_positionals() const;

    /** ites Element aller positionals (Nicht-Opts/Flags) zu double konvertiert
     * Ausnahme, falls ites Element nicht existiert
     * @param idx das wievielte Element, ites
     * @return iter Wert der Positionals als double
     */
    double double_pos(size_t idx) const;

    /** ites Element aller positionals (Nicht-Opts/Flags) zu double konvertiert
     * @param idx das wievielte Element, ites
     * @param defval Vorgabewert, falls ites nicht existiert
     * @return iter Wert der Positionals als double oder defval
     */
    double double_pos(size_t idx, double defval) const;

private:
    void init(int argc, char* argv[]); // constructor delegate
};

/** Erzeugt ein Feld von Zufallszahlen
 * @param how_many wie viele Zufallszahlen
 * @param lower kleinste Zahl, default -1048576
 * @param upper größte Zahl, default 1048575
 * @return ints genau howmany Zufallszahlen
 */
std::vector<int> create_randints(int how_many, int lower = -1048576,
                                 int upper = 1048575);

/** Erzeugt ein Feld von Zufallszahlen, immer die gleichen
 * (fester seed, zum testen)
 * @param how_many wie viele Zufallszahlen
 * @param lower kleinste Zahl, default -1048576
 * @param upper größte Zahl, default 1048575
 * @return ints genau howmany Zufallszahlen mit festem seed
 */
std::vector<int> create_same_randints(int how_many, int lower = -1048576,
                                      int upper = 1048575);

/** Sortierfunktion für ints
 */
typedef void (*sort_function)(std::vector<int>& a);

/** prüft, ob ein Feld aufsteigend sortiert ist
 *  @param a zu prüfendes Feld
 *  @param first_error falls nicht sortiert index des ersten
           falschen Elements, 0 sonst
 *  @return wahr gdw Feld ist aufsteigend sortiert
 */
bool is_sorted(std::vector<int>& a, int& first_error) noexcept;

/** Prüft eine Sortierfunktion mit Beispielen, mit Ausgaben
 * @param sort die zu testende Funktion
 * @param timing Zeitmessung, default false
 * @param large große Probleme, default false
 * @param verbose wie gesprächig, default 1
 * @return wahr gdw alle Tests das jeweilige Feld sortiert haben
 */
bool check_sort(sort_function sort, bool timing = false, bool large = false,
                int verbose = 1);

/** Prüft eine Sortierfunktion mit einem Beispiel fester Größe, mit Ausgaben
 * @param sort die zu testende Funktion
 * @param size wie groß soll das Beispiel sein
 * @param timing Zeitmessung, default false
 * @param verbose wie gesprächig, default 1
 * @return wahr gdw der Test das jeweilige Feld sortiert hat
 */
bool check_sort_one(sort_function sort, int size, bool timing = false,
                    int verbose = 1);

extern int _swaps_counter;
/** swap von zwei Elementen, zählt wie häufig gerufen
 * @param a Feld in dem geswapped wird
 * @param i eine Stelle, die geswapped wird
 * @param j andere Stelle, die geswapped wird
 * ignore possible sign errors
 */
inline void swap(std::vector<int>& a, int i, int j) noexcept {
    int h = a[static_cast<size_t>(i)];
    a[static_cast<size_t>(i)] = a[static_cast<size_t>(j)];
    a[static_cast<size_t>(j)] = h;
    _swaps_counter += 1;
}

/** swap von zwei Elementen in zwei Feldern, zählt wie häufig gerufen
 * @param a Feld in dem geswapped wird
 * @param i Stelle in a, die geswapped wird
 * @param b Feld in dem geswapped wird
 * @param j Stelle in b, die geswapped wird
 * ignore possible sign errors
 */
inline void swap(std::vector<int>& a, int i, std::vector<int>& b, int j) {
    int h = a[static_cast<size_t>(i)];
    a[static_cast<size_t>(i)] = b[static_cast<size_t>(j)];
    b[static_cast<size_t>(j)] = h;
    _swaps_counter += 1;
}

/** wie oft wurde swap gerufen seit letztem reset (nicht threadsafe)
 * @return wie oft swap gerufen
 */
int get_swaps() noexcept;

/** wie oft wurde swap gerufen seit letztem reset (nicht threadsafe)
 * und auf 0 setzen
 * @return wie oft swap gerufen
 */
int reset_swaps() noexcept;

/** Timer, um verbrauchte CPU-Zeit zu messen
 */
class Timer {
private:
    std::chrono::system_clock::time_point start;

public:
    Timer() noexcept;
    double measure() const noexcept;  //> in Sekunden
    long measure_ms() const noexcept; //> in Millisekunden
    long measure_us() const noexcept; //> in Mikrosekunden
    long measure_ns() const noexcept; //> in Nanosekunden
    void restart() noexcept; // Neustart
    // messen und als für Menschen lesbarer String formatieren
    std::string human_measure() const;
    // für Menschen lesbarer String formatieren aus double Zeit in Sekunden
    static std::string human_format(double secs);
};

/** Schlafe ein paar Millisekunden
 * @param ms wieviele Millisekunden schlafen
 */
void schlafe_ms(int ms);

/** Schlafe ein paar Mikrosekunden
 * @param us wieviele Mikrosekunden schlafen
 */
void schlafe_us(int us);

/** Ausgabe von vector<T>, default eines pro Zeile
 * @param tvec der Vector von Ts
 * @param per_line wie viele Elemente je Zeile, default 1
 * @param width Mindestbreite, default 0 (keine)
 * @param show_linenum Angabe der Zeilennummer: default nein
 */
template <typename T>
void show_tvec(const std::vector<T>& tvec, int per_line = 1, int width = 0,
               bool show_linenum = false) {
    int i = 0;
    int line_number = 0;
    constexpr char separator = ' ';
    const int tvec_size = static_cast<int>(tvec.size());
    const int len_lines = (tvec_size + (per_line - 1)) / per_line;
    int line_number_width =
        static_cast<int>(std::to_string(len_lines).length());
    for (const T val : tvec) {
        if (i % per_line == 0) {
            if (i > 0) {
                std::cout << std::endl;
            }
            line_number += 1;
            if (show_linenum) {
                std::cout << std::setw(line_number_width) << line_number;
                std::cout << ": ";
            }
        }
        if (width > 0) {
            std::cout << std::setw(width);
        }
        std::cout << val << separator;
        i += 1;
    }
    if (!tvec.empty()) {
        std::cout << std::endl;
    }
}


/** beliebig lange Ganzzahlen, langsam aber einfach zu verwenden
 * Python-Verhalten bei Division mit negativen Zahlen (/%)
 * Dezimal-Basis mit Zeichenketten (string), separates Vorzeichen.
 */
class BigInt {
private:
    std::string digits; // Dezimaldarstellung, z.B. "12345"
    int sign; // -1 or 1
    void init(unsigned long long);
public:

    /** Default Konstruktor, immer mit "0" initialisiert
     */
    BigInt() : BigInt(0) {}

    /* Konvertierungskonstruktor aus int
     * @param val int-Wert
     */
    BigInt(int val) : BigInt(static_cast<long long>(val)) {}

    /* Konvertierungskonstruktor aus long
     * @param val long-Wert
     */
    BigInt(long val) : BigInt(static_cast<long long>(val)) {}

    /* Konvertierungskonstruktor aus long long
     * @param val "long long"-Wert
     */
    BigInt(long long val);

    /* Konvertierungskonstruktor aus unsigned int
     * @param val "unsigned int"-Wert
     */
    BigInt(unsigned int val) : BigInt(static_cast<unsigned long long>(val)) {}

    /* Konvertierungskonstruktor aus unsigned long
     * @param val "unsigned long"-Wert
     */
    BigInt(unsigned long val) : BigInt(static_cast<unsigned long long>(val)) {}

    /* Konvertierungskonstruktor aus unsigned long long
     * @param val "unsigned long long"-Wert
     */
    BigInt(unsigned long long val) {
        init(val);
    }

    /* Konstruktor aus string von Ziffern, keine implizite Konvertierung
     * @param val string-Wert, darf nur aus +/- am Anfang 
                  und Dezimalziffern bestehen                  
     * @throws bei Eingabefehlern pbma_exception
     */
    explicit BigInt(const std::string& digits);

    /* Konstruktor aus double, keine implizite Konvertierung,
     * der ganzzahlige Teil, der in long long passt, wird übernommen
     * @param val 
     */
    explicit BigInt(double val) : BigInt(static_cast<long long>(val)) {}
    
    /** Ganzzahlige Addition ohne Überlauf
     * @param zu addierende Ganzzahl
     * @return Geändertes ganzzahliges Objekt
     */
    BigInt& operator+=(const BigInt&);

    /** Ganzzahlige Subtraktion ohne Überlauf
     * @param zu subtrahierende Ganzzahl
     * @return Geändertes ganzzahliges Objekt
     */
    BigInt& operator-=(const BigInt&);

    /** Ganzzahlige Multiplikation ohne Überlauf
     * @param zu multiplizierende Ganzzahl
     * @return Geändertes ganzzahliges Objekt
     */
    BigInt& operator*=(const BigInt&);

    /** Ganzzahlige Division, bei negativen Zahlen wie bei Python, 
        also immer weg von der Null 
        Bsp: 1000 /= 3 -> 333; -1000 /= 3 -> -334
     * @param ganzzahliger Teiler
     * @return Geändertes ganzzahliges Objekt
     */
    BigInt& operator/=(const BigInt&); 

    /** Ganzzahlige Modulo-Operation, das Ergebis ist wie bei Python
        positivem Modulus immer > 0 und bei negativem Modulus
        immer < 0, falls der Wert nicht 0 ist
     * @param ganzzahliger Modulus
     * @return Geändertes ganzzahliges Objekt
     */
    BigInt& operator%=(const BigInt&); // always positive, python %

    /** Vergleichsfunktion
     * @param zu vergleichende Ganzzahl
     * @return -1 wenn Zahl kleiner als zu vergleichende Zahl, 
                0 wenn gleich, 1 wenn größer
     */    
    int compare(const BigInt&) const;

    /** Vergleichsfunktion
     * @param zu vergleichende Ganzzahl
     * @return true, genau dann wenn gleich, genau dann wenn compare 0 ergibt
     */    
    bool equals(const BigInt&) const;

    /** Prefix-Inkrementieren
     * @return Geändertes ganzzahliges Objekt
     */
    BigInt& operator++() {
        operator+=(BigInt{1});
        return *this;
    }

    /** Postfix-Inkrementieren
     * @return Kopie, des alten ganzzahliges Objekts
     */
    BigInt operator++(int) {
        BigInt ret = *this;
        operator+=(BigInt{1});
        return ret;
    }

    /** Prefix-Dekrementieren
     * @return Geändertes ganzzahliges Objekt
     */
    BigInt& operator--() {
        operator-=(BigInt{1});
        return *this;
    }

    /** Postfix-Dekrementieren
     * @return Kopie, des alten ganzzahliges Objekts
     */
    BigInt operator--(int) {
        BigInt ret = *this;
        operator-=(BigInt{1});
        return ret;
    }
    
    friend std::ostream& operator<<(std::ostream& out, const BigInt& bi);
    friend std::istream& operator>>(std::istream& in, BigInt& bi);
    friend std::string to_string(const BigInt&);

    /** Zurückkonvertierung zu einer long-Zahl, 
     * @throws pbma_exception, wenn Zahlbereich überschritten
     */
    long to_long() const; 

    /** Zurückkonvertierung zu einer long long-Zahl, 
     * @throws pbma_exception, wenn Zahlbereich überschritten
     */
    long long to_long_long() const;

    /** Zurückkonvertierung zu einer double Zahl mit Typkonvertierung 
     * @throws pbma_exception, wenn Zahlbereich überschritten
     */
    double to_double() const { return static_cast<double>(to_long_long()); }
    
    operator int() const { return static_cast<int>(to_long()); }
    operator long() const { return to_long(); }
    operator double() const { return to_double(); }
};

/** Ausgabe-Operator, niemals + am Anfang, - bei negativer Zahl, 
    immer die Dezimaldarstellung
 * @param der Ausgabestrom
 * @param die auszugebende Ganzzahl
 * @return der übergebene Strom
 */ 
std::ostream& operator<<(std::ostream& out, const BigInt&);

/** Eingabe-Operator, + oder - am Anfang einmal optional, gefolgt von 
    beliebig vielen Dezimalzeichen, Ausgabestrom wird fail bei Fehler
 * @param der Ausgabestrom
 * @param die auszugebende Ganzzahl
 * @return der übergebene Strom
 */ 
std::istream& operator>>(std::istream& in, BigInt&);

/** Konvertierung zu einem string, wie Ausgabeoperator
 * @param die zu konvertierende Ganzzahl
 */
std::string to_string(const BigInt&);

/** Plus-Operator für beliebig lange Zahlen
 * @param Ganzzahl
 * @param Ganzzahl
 * @return Summe der beiden Zahlen
 */ 
inline BigInt operator+(const BigInt& self, const BigInt& other) {
    BigInt ret = self;
    ret += other;
    return ret;
}
inline BigInt operator+(const BigInt& self, long other) {
    return self + BigInt(other);
}
inline BigInt operator+(const BigInt& self, int other) {
    return self + BigInt(other);
}
inline BigInt operator+(long self, const BigInt& other) {
    return BigInt(self) + other;
}
inline BigInt operator+(int self, const BigInt& other) {
    return BigInt(self) + other;
}

/** Minus-Operator für beliebig lange Zahlen
 * @param Ganzzahl
 * @param Ganzzahl
 * @return Erste minus die zweite Zahl
 */ 
inline BigInt operator-(const BigInt& self, const BigInt& other) {
    BigInt ret = self;
    ret -= other;
    return ret;
}
inline BigInt operator-(const BigInt& self, long other) {
    return self - BigInt(other);
}
inline BigInt operator-(const BigInt& self, int other) {
    return self - BigInt(other);
}
inline BigInt operator-(long self, const BigInt& other) {
    return BigInt(self) - other;
}
inline BigInt operator-(int self, const BigInt& other) {
    return BigInt(self) - other;
}

/** Multiplikations-Operator für beliebig lange Zahlen
 * @param Ganzzahl
 * @param Ganzzahl
 * @return Produkt der beiden Zahlen
 */ 
inline BigInt operator*(const BigInt& self, const BigInt& other) {
    BigInt ret = self;
    ret *= other;
    return ret;
}
inline BigInt operator*(const BigInt& self, long other) {
    return self * BigInt(other);
}
inline BigInt operator*(const BigInt& self, int other) {
    return self * BigInt(other);
}
inline BigInt operator*(long self, const BigInt& other) {
    return BigInt(self) * other;
}
inline BigInt operator*(int self, const BigInt& other) {
    return BigInt(self) * other;
}

/** Divisions-Operator für beliebig lange Zahlen, siehe /=
 * @param Ganzzahl
 * @param Ganzzahl
 * @return Erste durch die zweite Zahl
 */ 
inline BigInt operator/(const BigInt& self, const BigInt& other) {
    BigInt ret = self;
    ret /= other;
    return ret;
}
inline BigInt operator/(const BigInt& self, long other) {
    return self / BigInt(other);
}
inline BigInt operator/(const BigInt& self, int other) {
    return self / BigInt(other);
}
inline BigInt operator/(long self, const BigInt& other) {
    return BigInt(self) / other;
}
inline BigInt operator/(int self, const BigInt& other) {
    return BigInt(self) / other;
}

/** Modulo-Operator für beliebig lange Zahlen, siehe %=
 * @param Ganzzahl
 * @param Ganzzahl
 * @return Erste modulo die zweite Zahl
 */ 
inline BigInt operator%(const BigInt& self, const BigInt& other) {
    BigInt ret = self;
    ret %= other;
    return ret;
}
inline BigInt operator%(const BigInt& self, long other) {
    return self % BigInt(other);
}
inline BigInt operator%(const BigInt& self, int other) {
    return self % BigInt(other);
}
inline BigInt operator%(long self, const BigInt& other) {
    return BigInt(self) % other;
}
inline BigInt operator%(int self, const BigInt& other) {
    return BigInt(self) % other;
}

/** Kleiner-Operator für beliebig lange Zahlen
 * @param Ganzzahl
 * @param Ganzzahl
 * @return true gdw Erste ist echt kleiner als die zweite Zahl
 */ 
inline bool operator<(const BigInt& self, const BigInt& other) {
    return self.compare(other) < 0;
}
inline bool operator<(const BigInt& self, long other) {
    return self.compare(BigInt(other)) < 0;
}
inline bool operator<(const BigInt& self, int other) {
    return self.compare(BigInt(other)) < 0;
}
inline bool operator<(int self, const BigInt& other) {
    return other.compare(BigInt(self)) > 0;
}
inline bool operator<(long self, const BigInt& other) {
    return other.compare(BigInt(self)) > 0;
}

/** Kleinergleich-Operator für beliebig lange Zahlen
 * @param Ganzzahl
 * @param Ganzzahl
 * @return true gdw Erste ist kleiner gleich die zweite Zahl
 */ 
inline bool operator<=(const BigInt& self, const BigInt& other) {
    return self.compare(other) <= 0;
}
inline bool operator<=(const BigInt& self, long other) {
    return self.compare(BigInt(other)) <= 0;
}
inline bool operator<=(const BigInt& self, int other) {
    return self.compare(BigInt(other)) <= 0;
}
inline bool operator<=(int self, const BigInt& other) {
    return other.compare(BigInt(self)) >= 0;
}
inline bool operator<=(long self, const BigInt& other) {
    return other.compare(BigInt(self)) >= 0;
}

/** Groesser-Operator für beliebig lange Zahlen
 * @param Ganzzahl
 * @param Ganzzahl
 * @return true gdw Erste ist echt groesser als die zweite Zahl
 */ 
inline bool operator>(const BigInt& self, const BigInt& other) {
    return self.compare(other) > 0;
}
inline bool operator>(const BigInt& self, long other) {
    return self.compare(BigInt(other)) > 0;
}
inline bool operator>(const BigInt& self, int other) {
    return self.compare(BigInt(other)) > 0;
}
inline bool operator>(int self, const BigInt& other) {
    return other.compare(BigInt(self)) < 0;
}
inline bool operator>(long self, const BigInt& other) {
    return other.compare(BigInt(self)) < 0;
}

/** Groessergleich-Operator für beliebig lange Zahlen
 * @param Ganzzahl
 * @param Ganzzahl
 * @return true gdw Erste ist groesser gleich die zweite Zahl
 */ 
inline bool operator>=(const BigInt& self, const BigInt& other) {
    return self.compare(other) >= 0;
}
inline bool operator>=(const BigInt& self, long other) {
    return self.compare(BigInt(other)) >= 0;
}
inline bool operator>=(const BigInt& self, int other) {
    return self.compare(BigInt(other)) >= 0;
}
inline bool operator>=(int self, const BigInt& other) {
    return other.compare(BigInt(self)) <= 0;
}
inline bool operator>=(long self, const BigInt& other) {
    return other.compare(BigInt(self)) <= 0;
}

/** Gleichheits-Operator für beliebig lange Zahlen
 * @param Ganzzahl
 * @param Ganzzahl
 * @return true gdw Erste ist gleich der zweiten Zahl
 */ 
inline bool operator==(const BigInt& self, const BigInt& other) {
    return self.equals(other);
}
inline bool operator==(const BigInt& self, long other) {
    return self.equals(BigInt(other));
}
inline bool operator==(const BigInt& self, int other) {
    return self.equals(BigInt(other));
}
inline bool operator==(int self, const BigInt& other) {
    return other.equals(BigInt(self));
}
inline bool operator==(long self, const BigInt& other) {
    return other.equals(BigInt(self));
}

/** Ungleichheits-Operator für beliebig lange Zahlen
 * @param Ganzzahl
 * @param Ganzzahl
 * @return true gdw Erste ist ungleich der zweiten Zahl
 */ 
inline bool operator!=(const BigInt& self, const BigInt& other) {
    return !self.equals(other);
}
inline bool operator!=(const BigInt& self, long other) {
    return !self.equals(BigInt(other));
}
inline bool operator!=(const BigInt& self, int other) {
    return !self.equals(BigInt(other));
}
inline bool operator!=(int self, const BigInt& other) {
    return !other.equals(BigInt(self));
}
inline bool operator!=(long self, const BigInt& other) {
    return !other.equals(BigInt(self));
}


/** &tldr; ot_swap statt swap, dann ist es nicht mehr rot
 * Der Indexer von CDT (der C++ Erweiterung von Eclipse) ist
 * entsetzlich, hat eine Menge Bugs und wirft Fehler bei modernem C++.
 * Ein nerviges Problem ist, dass swap immer als Fehler angezeigt wird,
 * da der Indexer nicht in der Lage ist, den komplexen Code der
 * Standard-Bibliothek zu verstehen. Wenn man den allerdings in einen
 * Wrapper steckt (eclipse-2021-12), dann merkt er das nicht.
 */
template <typename T>
void ot_swap(T& t1, T& t2) {
    std::swap(t1, t2);
}

#endif
