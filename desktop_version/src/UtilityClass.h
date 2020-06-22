#ifndef UTILITYCLASS_H
#define UTILITYCLASS_H

#include <SDL.h>
#include <vector>
#include <string>

template<typename T>
class growing_vector : public std::vector<T> {
    public:
    template<typename Dummy = void>
    growing_vector<T>(const std::vector<T>& v) : std::vector<T>(v) {}
    using std::vector<T>::vector;

    template<typename T2 = T>
    T2 const& operator[](typename std::vector<T2>::size_type index) const {
        typename std::vector<T>::size_type needed_size = index + 1;
        if (this->size() < needed_size) {
            this->resize(needed_size);
        }
        return std::vector<T>::operator[](index);
    }
    template<typename T2 = T>
    T2& operator[](typename std::vector<T2>::size_type index) {
        typename std::vector<T>::size_type needed_size = index + 1;
        if (this->size() < needed_size) {
            this->resize(needed_size);
        }
        return std::vector<T>::operator[](index);
    }
};

// always return positive modulo result if modulus is positive
template<typename N>
N mod(const N &num, const N &mod) {
    return (num % mod + mod) % mod;
}

int ss_toi(std::string _s);
int relativepos(int original, std::string parsethis);
void relativepos(int* original, std::string parsethis);
bool relativebool(bool original, std::string parsethis);
void relativebool(bool* original, std::string parsethis);

bool is_number(const std::string& s);

bool parsebool(std::string parsethis);

bool is_positive_num(const std::string& str, bool hex);

growing_vector<std::string> split(const std::string &s, char delim, growing_vector<std::string> &elems);

growing_vector<std::string> split(const std::string &s, char delim);

#define INBOUNDS(index, vector) ((int) index >= 0 && (int) index < (int) vector.size())


//helperClass
class UtilityClass
{
public:
    UtilityClass();

    static std::string String(int _v);

    static std::string GCString(growing_vector<SDL_GameControllerButton> buttons);

    std::string twodigits(int t);

    std::string timestring(int t);

    std::string number(int _t);


    static bool intersects( SDL_Rect A, SDL_Rect B );

    void updateglow();

    std::string getmusicname(int num);

    int glow = 0;
    bool freezeglow = false;
    int slowsine = 0;
    int glowdir = 0;
    int globaltemp = 0;
    int temp = 0;
    int temp2 = 0;
    growing_vector<int> splitseconds;
};

extern UtilityClass help;

#endif /* UTILITYCLASS_H */
