#ifndef ITU_GRAPHICS_PROGRAMMING_COLOR_H
#define ITU_GRAPHICS_PROGRAMMING_COLOR_H

#include "glad/glad.h"

struct Color
{
    GLubyte r;
    GLubyte g;
    GLubyte b;
    GLubyte a;

    Color( GLubyte r, GLubyte g, GLubyte b, GLubyte a )
    {
        this->r = r;
        this->g = g;
        this->b = b;
        this->a = a;
    }

    Color( GLubyte r, GLubyte g, GLubyte b )
    {
        this->r = r;
        this->g = g;
        this->b = b;
        a = 255;
    }

    // Override equality operator
    bool operator==( const Color &other ) const
    {
        return r == other.r && g == other.g && b == other.b && a == other.a;
    }
};

#endif //ITU_GRAPHICS_PROGRAMMING_COLOR_H
