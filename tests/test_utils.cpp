#include <cassert>
#include <cmath>
#include <cstdio>

#include "utils/utils.h"

static bool floatNear(float a, float b, float eps)
{
    return std::fabs(a - b) <= eps;
}

static void testMathAndRng(void)
{
    b2Vec2 a = { 3.0f, 4.0f };
    b2Vec2 b = { 1.0f, -2.0f };

    assert(floatNear(math::Dot(a, b), -5.0f, 1e-5f));
    assert(floatNear(math::Det(a, b), -10.0f, 1e-5f));
    assert(floatNear(math::Distance(b2Vec2_zero, a), 5.0f, 1e-5f));
    assert(floatNear(math::DistanceSquared(b2Vec2_zero, a), 25.0f, 1e-5f));

    assert(math::Min(3, 7) == 3);
    assert(math::Clamp(-2, 0, 5) == 0);
    assert(math::Clamp(8, 0, 5) == 5);

    for (int i = 0; i < 50; i++)
    {
        int r = rng::RangeI(2, 6);
        assert(2 <= r && r <= 6);
        float rf = rng::RangeF(1.0f, 3.0f);
        assert(1.0f <= rf && rf <= 3.0f);
    }

    std::printf("[OK] testMathAndRng\n");
}

int main(void)
{
    testMathAndRng();
    std::printf("All tests passed.\n");
    return 0;
}

