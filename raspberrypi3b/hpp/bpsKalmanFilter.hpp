
#pragma once

#include <iostream>

class KalmanFilter
{
    private:
        float KG, Eest, Eest1, Emeas, EST, EST1, MEA; 
        float CalKG(float Eest1, float Emeas);
        float CalEST(float EST1, float KG, float MEA);
        float CalEest(float KG, float Eest1);
    public:
        KalmanFilter(float EST1, float Eest1, float Emeas)
            : EST1(EST1), Eest1(Eest1), Emeas(Emeas){};
        float predict(float MEA);
};
