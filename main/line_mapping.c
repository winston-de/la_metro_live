#include <stdio.h>
#include "line_mapping.h"

// static Station E_STOPS[] = {
//     {80401, {0, 139}},     // Atlantic Station
//     {80402, {1, 138}},     // East LA Civic Center
//     {80403, {2, 137}},     // Maravilla
//     {80404, {3, 136}},     // Indiana
//     {80405, {4, 135}},     // Soto
//     {80406, {5, 134}},     // Mariachi Plaza
//     {80407, {6, 133}},     // Pico / Aliso
//     {81403, {53, 132}},     // Little Tokyo / Arts District
//     {81402, {54, 131}},     // Historic Broadway
//     {81401, {55, 130}},     // Grand Ave Arts / Bunker Hill
//     {80122, {56, 129}},     // 7th Street / Metro Center
//     {80121, {57, 128}},     // Pico
//     {80123, {58, 91}},     // LATTC / Ortho Institute
//     {80124, {59, 90}},     // Jefferson / USC
//     {80125, {60, 89}},     // Expo Park / USC
//     {80126, {61, 88}},     // Expo / Vermont
//     {80127, {62, 87}},     // Expo / Western
//     {80128, {63, 86}},     // Expo / Crenshaw
//     {80129, {64, 85}},     // Farmdale
//     {80130, {65, 84}},     // Expo / La Brea
//     {80131, {66, 83}},     // La Cienega / Jefferson
//     {80132, {67, 82}},     // Culver City
//     {80133, {68, 81}},     // Palms
//     {80134, {69, 80}},     // Westwood / Rancho Park
//     {80135, {70, 79}},     // Expo / Sepulveda
//     {80136, {71, 78}},     // Expo / Bundy
//     {80137, {72, 77}},     // 26th Street / Bergamot
//     {80138, {73, 76}},     // 17th Street / SMC
//     {80139, {74, 75}}      // Downtown Santa Monica
// };

// static Station A_STOPS[] = {
//     {801103, {30, 29}},     // Pomona North
//     {801102, {31, 28}},     // La Verne / Fairplex
//     {801101, {32, 27}},     // San Dimas Station
//     {801100, {33, 26}},     // Glendora
//     {80427, {34, 25}},     // APU / Citrus College Station
//     {80426, {35, 24}},     // Azusa Downtown
//     {80425, {36, 23}},     // Irwindale
//     {80424, {37, 22}},     // Duarte / City of Hope
//     {80423, {38, 21}},     // Monrovia
//     {80422, {39, 20}},     // Arcadia
//     {80421, {40, 19}},     // Sierra Madre Villa
//     {80420, {41, 18}},     // Allen
//     {80419, {42, 17}},     // Lake
//     {80418, {43, 16}},     // Memorial Park
//     {80417, {44, 15}},     // Del Mar
//     {80416, {45, 14}},     // Fillmore
//     {80415, {46, 13}},     // South Pasadena
//     {80414, {47, 12}},     // Highland Park
//     {80413, {48, 11}},     // Southwest Museum
//     {80412, {49, 10}},     // Heritage Square
//     {80411, {50, 9}},     // Lincoln / Cypress
//     {80410, {51, 8}},     // Chinatown
//     {80409, {52, 7}},     // Union Station
//     {81403, {53, 132}},     // Little Tokyo / Arts District
//     {81402, {54, 131}},     // Historic Broadway
//     {81401, {55, 130}},     // Grand Ave Arts / Bunker Hill
//     {80122, {56, 129}},     // 7th Street / Metro Center
//     {80121, {57, 128}},     // Pico
//     {80120, {92, 127}},     // Grand / LATTC
//     {80119, {93, 126}},     // San Pedro Street
//     {80118, {94, 125}},     // Washington
//     {80117, {95, 124}},     // Vernon
//     {80116, {96, 123}},     // Slauson
//     {80115, {97, 122}},     // Florence
//     {80114, {98, 121}},     // Firestone
//     {80113, {99, 120}},     // 103rd Street / Watts Towers
//     {80112, {100, 119}},     // Willowbrook - Rosa Parks Station
//     {80111, {101, 118}},     // Compton
//     {80110, {102, 117}},     // Artesia
//     {80109, {103, 116}},     // Del Amo
//     {80108, {104, 115}},     // Wardlow Station
//     {80107, {105, 114}},     // Willow Street
//     {80106, {106, 113}},     // Pacific Coast Hwy
//     {80105, {107, 112}},     // Anaheim Street
//     {80154, {108, 255}},     // 5th Street Station
//     {80153, {109, 255}},     // 1st Street
//     {80101, {110, 255}},     // Downtown Long Beach Station
//     {80102, {111, 255}},     // Pacific Ave Station
// };

// static Station B_STOPS[] = {
//     {80214, {3, 2}},     // Union Station
//     {80213, {4, 1}},     // Civic Center / Grand Park
//     {80212, {5, 0}},     // Pershing Square Station
//     {80211, {6, 45}},     // 7th Street / Metro Center Station
//     {80210, {7, 44}},     // Westlake / MacArthur Park Station
//     {80209, {8, 43}},     // Wilshire / Vermont Station
//     {80208, {9, 24}},     // Vermont / Beverly Station
//     {80207, {10, 23}},     // Vermont / Santa Monica
//     {80206, {11, 22}},     // Vermont / Sunset
//     {80205, {12, 21}},     // Hollywood / Western
//     {80204, {13, 20}},     // Hollywood / Vine
//     {80203, {14, 19}},     // Hollywood / Highland
//     {80202, {15, 18}},     // Universal / Studio City
//     {80201, {16, 17}},     // North Hollywood
// };

// static Station D_STOPS[] = {
//     {80214, {3, 2}},     // Union Station
//     {80213, {4, 1}},     // Civic Center / Grand Park
//     {80212, {5, 0}},     // Pershing Square Station
//     {80211, {6, 45}},     // 7th Street / Metro Center Station
//     {80210, {7, 44}},     // Westlake / MacArthur Park Station
//     {80209, {8, 43}},     // Wilshire / Vermont Station
//     {80215, {25, 42}},     // Wilshire / Normandie
//     {80216, {26, 41}},     // Wilshire / Western
// };

// static Station C_STOPS[] = {
//     {80314, {0, 45}},     // Norwalk Station
//     {80313, {1, 44}},     // Lakewood Blvd
//     {80312, {2, 43}},     // Lynwood
//     {80311, {3, 42}},     // Willowbrook - Rosa Parks
//     {80310, {4, 41}},     // Avalon
//     {80309, {5, 40}},     // Harbor Freeway
//     {80308, {6, 39}},     // Vermont / Athens
//     {80307, {7, 38}},     // Crenshaw
//     {80306, {8, 37}},     // Hawthorne / Lennox
//     {80305, {9, 36}},     // Aviation / Imperial
//     {80701, {10, 27}},     // Aviation / Century
//     {80702, {11, 26}},     // LAX / Metro Transit Center
// };

// static Station K_STOPS[] = {
//     {80301, {32, 31}},     // Redondo Beach
//     {80302, {33, 30}},     // Douglas
//     {80303, {34, 29}},     // El Segundo
//     {80304, {35, 28}},     // Mariposa Station
//     {80701, {10, 27}},     // Aviation / Century Station
//     {80702, {11, 26}},     // LAX / Metro Transit Center
//     {80703, {12, 25}},     // Westchester / Veterans
//     {80704, {13, 24}},     // Downtown Inglewood
//     {80705, {14, 23}},     // Fairview Heights
//     {80706, {15, 22}},     // Hyde Park
//     {80707, {16, 21}},     // Leimert Park
//     {80708, {17, 20}},     // Martin Luther King Jr
//     {80709, {18, 19}},     // Expo / Crenshaw K-Line
// };

static Station AE_Stops[] = {
    {80101, {110, 255}}, // Downtown Long Beach Station
    {80102, {111, 255}}, // Pacific Ave Station
    {80105, {107, 112}}, // Anaheim Street
    {80106, {106, 113}}, // Pacific Coast Hwy
    {80107, {105, 114}}, // Willow Street
    {80108, {104, 115}}, // Wardlow Station
    {80109, {103, 116}}, // Del Amo
    {80110, {102, 117}}, // Artesia
    {80111, {101, 118}}, // Compton
    {80112, {100, 119}}, // Willowbrook - Rosa Parks Station
    {80113, {99, 120}},  // 103rd Street / Watts Towers
    {80114, {98, 121}},  // Firestone
    {80115, {97, 122}},  // Florence
    {80116, {96, 123}},  // Slauson
    {80117, {95, 124}},  // Vernon
    {80118, {94, 125}},  // Washington
    {80119, {93, 126}},  // San Pedro Street
    {80120, {92, 127}},  // Grand / LATTC
    {80121, {57, 128}},  // Pico
    {80122, {56, 129}},  // 7th Street / Metro Center
    {80123, {58, 91}},   // LATTC / Ortho Institute
    {80124, {59, 90}},   // Jefferson / USC
    {80125, {60, 89}},   // Expo Park / USC
    {80126, {61, 88}},   // Expo / Vermont
    {80127, {62, 87}},   // Expo / Western
    {80128, {63, 86}},   // Expo / Crenshaw
    {80129, {64, 85}},   // Farmdale
    {80130, {65, 84}},   // Expo / La Brea
    {80131, {66, 83}},   // La Cienega / Jefferson
    {80132, {67, 82}},   // Culver City
    {80133, {68, 81}},   // Palms
    {80134, {69, 80}},   // Westwood / Rancho Park
    {80135, {70, 79}},   // Expo / Sepulveda
    {80136, {71, 78}},   // Expo / Bundy
    {80137, {72, 77}},   // 26th Street / Bergamot
    {80138, {73, 76}},   // 17th Street / SMC
    {80139, {74, 75}},   // Downtown Santa Monica
    {80153, {109, 255}}, // 1st Street
    {80154, {108, 255}}, // 5th Street Station
    {80401, {0, 139}},   // Atlantic Station
    {80402, {1, 138}},   // East LA Civic Center
    {80403, {2, 137}},   // Maravilla
    {80404, {3, 136}},   // Indiana
    {80405, {4, 135}},   // Soto
    {80406, {5, 134}},   // Mariachi Plaza
    {80407, {6, 133}},   // Pico / Aliso
    {80409, {52, 7}},    // Union Station
    {80410, {51, 8}},    // Chinatown
    {80411, {50, 9}},    // Lincoln / Cypress
    {80412, {49, 10}},   // Heritage Square
    {80413, {48, 11}},   // Southwest Museum
    {80414, {47, 12}},   // Highland Park
    {80415, {46, 13}},   // South Pasadena
    {80416, {45, 14}},   // Fillmore
    {80417, {44, 15}},   // Del Mar
    {80418, {43, 16}},   // Memorial Park
    {80419, {42, 17}},   // Lake
    {80420, {41, 18}},   // Allen
    {80421, {40, 19}},   // Sierra Madre Villa
    {80422, {39, 20}},   // Arcadia
    {80423, {38, 21}},   // Monrovia
    {80424, {37, 22}},   // Duarte / City of Hope
    {80425, {36, 23}},   // Irwindale
    {80426, {35, 24}},   // Azusa Downtown
    {80427, {34, 25}},   // APU / Citrus College Station
    {81401, {55, 130}},  // Grand Ave Arts / Bunker Hill
    {81402, {54, 131}},  // Historic Broadway
    {81403, {53, 132}},  // Little Tokyo / Arts District
    {801100, {33, 26}},  // Glendora
    {801101, {32, 27}},  // San Dimas Station
    {801102, {31, 28}},  // La Verne / Fairplex
    {801103, {30, 29}},  // Pomona North
};

static Station BD_Stops[] = {
    {80201, {17, 16}}, // North Hollywood
    {80202, {18, 15}}, // Universal / Studio City
    {80203, {19, 14}}, // Hollywood / Highland
    {80204, {20, 13}}, // Hollywood / Vine
    {80205, {21, 12}}, // Hollywood / Western
    {80206, {22, 11}}, // Vermont / Sunset
    {80207, {23, 10}}, // Vermont / Santa Monica
    {80208, {24, 9}},  // Vermont / Beverly Station
    {80209, {43, 8}},  // Wilshire / Vermont Station
    {80210, {44, 7}},  // Westlake / MacArthur Park Station
    {80211, {45, 6}},  // 7th Street / Metro Center Station
    {80212, {0, 5}},   // Pershing Square Station
    {80213, {1, 4}},   // Civic Center / Grand Park
    {80214, {2, 3}},   // Union Station
    {80215, {25, 42}}, // Wilshire / Normandie
    {80216, {26, 41}}, // Wilshire / Western

};

static Station CK_Stops[] = {
    {80301, {31, 32}}, // Redondo Beach
    {80302, {30, 33}}, // Douglas
    {80303, {29, 34}}, // El Segundo
    {80304, {28, 35}}, // Mariposa Station
    {80305, {36, 9}},  // Aviation / Imperial
    {80306, {37, 8}},  // Hawthorne / Lennox
    {80307, {38, 7}},  // Crenshaw
    {80308, {39, 6}},  // Vermont / Athens
    {80309, {40, 5}},  // Harbor Freeway
    {80310, {41, 4}},  // Avalon
    {80311, {42, 3}},  // Willowbrook - Rosa Parks
    {80312, {43, 2}},  // Lynwood
    {80313, {44, 1}},  // Lakewood Blvd
    {80314, {45, 0}},  // Norwalk Station
    {80701, {27, 10}}, // Aviation / Century
    {80702, {26, 11}}, // LAX / Metro Transit Center
    {80703, {25, 12}}, // Westchester / Veterans
    {80704, {24, 13}}, // Downtown Inglewood
    {80705, {23, 14}}, // Fairview Heights
    {80706, {22, 15}}, // Hyde Park
    {80707, {21, 16}}, // Leimert Park
    {80708, {20, 17}}, // Martin Luther King Jr
    {80709, {19, 18}}, // Expo / Crenshaw K-Line

};

MLine* get_all_lines() {
    MLine lines[] = {
        {AE_Stops, 801, ARRL(AE_Stops), A_LED_VAL}, // A
        {BD_Stops, 802, ARRL(BD_Stops), B_LED_VAL}, // B
        {CK_Stops, 803, ARRL(BD_Stops), C_LED_VAL}, // C
        {BD_Stops, 805, ARRL(BD_Stops), D_LED_VAL}, // D
        {AE_Stops, 804, ARRL(AE_Stops), E_LED_VAL}, // E
        {CK_Stops, 807, ARRL(BD_Stops), K_LED_VAL}, // K
        
    };

    return lines;
}

// MLine get_e_line()
// {
//     MLine e_line = {
//         E_STOPS, 'E', ARRL(E_STOPS), E_LED_VAL};
//     return e_line;
// }

// MLine get_a_line()
// {
//     MLine a_line = {
//         A_STOPS, 'A', ARRL(A_STOPS), A_LED_VAL};
//     return a_line;
// }

// MLine get_b_line()
// {
//     MLine b_line = {
//         B_STOPS, 'B', ARRL(B_STOPS), B_LED_VAL};
//     return b_line;
// }

// MLine get_d_line()
// {
//     MLine d_line = {
//         D_STOPS, 'D', ARRL(D_STOPS), D_LED_VAL};
//     return d_line;
// }

// MLine get_c_line()
// {
//     MLine c_line = {
//         C_STOPS, 'C', ARRL(C_STOPS), C_LED_VAL};
//     return c_line;
// }

// MLine get_k_line()
// {
//     MLine k_line = {
//         K_STOPS, 'K', ARRL(K_STOPS), K_LED_VAL};

//     return k_line;
// }