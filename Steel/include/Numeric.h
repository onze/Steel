#ifndef STEEL_NUMERIC_H
#define STEEL_NUMERIC_H

namespace Steel
{
    class Numeric
    {
    public:
        template<typename T>
        static T clamp(T const& value, T const& min, T const& max)
        {
            return value < min ? min : (value > max ? max : value);
        }
        
        template<typename T>
        static T min(T const& x, T const& y)
        {
            return x < y ? x : y;
        }
        
        template<typename T>
        static T max(T const& x, T const& y)
        {
            return x < y ? y : x;
        }
        
    };
}

#endif // STEEL_NUMERIC_H
