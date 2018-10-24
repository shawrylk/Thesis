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
    KG = CalKG(Eest1, Emeas);
    EST1 = CalEST(EST1, KG, MEA);
    Eest1 = CalEest(KG, Eest1);
    return EST1;
}