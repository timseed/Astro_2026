#include <cstring>
using namespace std;

#include "Annotation.h"

Annotation::Annotation() : width_(0), height_(0)
{
}

Annotation::Annotation(const unsigned char color[3]) : width_(0), height_(0)
{
    memcpy(color_, color, 3);
}

Annotation::~Annotation()
{
}

