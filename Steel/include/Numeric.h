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
    };
}

#endif // STEEL_NUMERIC_H
