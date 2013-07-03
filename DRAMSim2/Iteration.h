#ifndef ITERATION_H
#define ITERATION_H

const int maxiter = 24;
const unsigned iterCdf_32[3][maxiter] = {
{45972,   80816,   101525,  118738,  125808,  128535,  129581,  130403, 130809,  130985,  131044,  131062,  131069,  131072,  131072,  131072, 131072, 131072, 131072, 131072,131072, 131072,131072, 131072},
{60579,  112739,  126555,  129478,  130330,  130943,  131046,  131067,  131070,  131071,  131072,  131072,  131072,  131072,  131072,  131072, 131072, 131072, 131072, 131072,131072, 131072,131072, 131072},
{74557,  117760,  126691,  129298,  130781,  131027,  131065,  131069,  131072,  131072,  131072,  131072,  131072,  131072,  131072,  131072, 131072, 131072, 131072, 131072,131072, 131072,131072, 131072,}};
const unsigned iterValue_32[3][maxiter] = {
{5,   6,   4,   7,   8,   9,   10 , 3,   11,  12,  13,  14,  15,  16,  1,   2, 17, 18, 19, 20, 21, 22, 23, 24},
{3,  4,   5,   6,   2,   7,   8,   9,   10,  11,  12,  1,   13,  14,  15,  16, 17, 18, 19, 20, 21, 22, 23, 24},
{3,  4,   5,   2,   6,   7,   8,   10,  9,   1,   11,  12,  13,  14,  15,  16, 17, 18, 19, 20, 21, 22, 23, 24}};    


const unsigned iterCdf_93[3][maxiter] = {
{44910, 81396, 99988, 117892, 125580, 128602, 129788, 130420, 130827, 130985, 131041, 131059, 131066, 131069, 131070, 131071, 131072, 131072, 131072, 131072, 131072, 131072, 131072, 131072},
{72563, 106871, 120952, 129037, 130682, 131007, 131057, 131067, 131070, 131072, 131072, 131072, 131072, 131072, 131072, 131072, 131072, 131072, 131072, 131072, 131072, 131072, 131072, 131072},
{74265, 102199, 125940, 130236, 130923, 131048, 131068, 131070, 131071, 131072, 131072, 131072, 131072, 131072, 131072, 131072, 131072, 131072, 131072, 131072, 131072, 131072, 131072, 131072}};

const unsigned iterValue_93[3][maxiter] = {
{6, 7, 8, 5, 9, 10, 11, 4, 12, 13, 14, 15, 16, 17, 18, 19, 20, 1, 2, 3, 21, 22, 23, 24},
{4, 5, 3, 6, 7, 8, 9, 10, 11, 12, 1, 2, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24},
{4, 3, 5, 6, 7, 8, 9, 10, 2, 11, 1, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24}, 
};

const unsigned iterCdf_256[3][maxiter] = {
{44523, 81494, 100305, 117540, 125523, 128513, 129722, 130346, 130806, 130970, 131036, 131060, 131065, 131068, 131069, 131070, 131071, 131072, 131072, 131072, 131072, 131072, 131072, 131072},
{63128, 104649, 125317, 129730, 130563, 130833, 131019, 131065, 131072, 131072, 131072, 131072, 131072, 131072, 131072, 131072, 131072, 131072, 131072, 131072, 131072, 131072, 131072, 131072},
{64548, 115331, 126860, 128861, 130680, 131015, 131060, 131070, 131071, 131072, 131072, 131072, 131072, 131072, 131072, 131072, 131072, 131072, 131072, 131072, 131072, 131072, 131072, 131072}};

const unsigned iterValue_256[3][maxiter] = {
{7, 8, 9, 6, 10, 11, 12, 5, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 1, 2, 3, 4, 23, 24},
{5, 4, 6, 7, 8, 3, 9, 10, 11, 1, 2, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24},
{4, 5, 6, 7, 3, 8, 9, 10, 11, 12, 1, 2, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24}};

#endif
