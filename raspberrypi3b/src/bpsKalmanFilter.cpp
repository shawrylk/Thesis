#include "../hpp/bpsKalmanFilter.hpp"

float KalmanFilter::CalKG(float Eest1, float Emeas)
{
    return (Eest1 / (Eest1 + Emeas));
}

float KalmanFilter::CalEST(float EST1, float KG, float MEA)
{
    return (EST1 + KG * ( MEA - EST1));
}

float KalmanFilter::CalEest(float KG, float Eest1)
{
    return ((1 - KG) * EST1);
}

float KalmanFilter::predict(float MEA)
{
    KG = CalKG(Eest, Emeas);
    std::cout << KG << "\n";
    EST1 = CalEST(EST1, KG, MEA);
    std::cout << EST1 << "\n";
    Eest1 = CalEest(KG, Eest1);
    std::cout << Eest1 << "\n";
    return EST1;
}