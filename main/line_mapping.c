#include <stdio.h>
#include <string.h>
#include "line_mapping.h"


#define NUM_LGND_LEDS 6
#define NUM_AE_LEDS 140
#define NUM_BD_LEDS 46
#define NUM_CK_LEDS 46

static Station AE_Stops[] = {
    {101, {110, 255}}, // Downtown Long Beach Station
    {102, {111, 255}}, // Pacific Ave Station
    {105, {107, 112}}, // Anaheim Street
    {106, {106, 113}}, // Pacific Coast Hwy
    {107, {105, 114}}, // Willow Street
    {108, {104, 115}}, // Wardlow Station
    {109, {103, 116}}, // Del Amo
    {110, {102, 117}}, // Artesia
    {111, {101, 118}}, // Compton
    {112, {100, 119}}, // Willowbrook - Rosa Parks Station
    {113, {99, 120}},  // 103rd Street / Watts Towers
    {114, {98, 121}},  // Firestone
    {115, {97, 122}},  // Florence
    {116, {96, 123}},  // Slauson
    {117, {95, 124}},  // Vernon
    {118, {94, 125}},  // Washington
    {119, {93, 126}},  // San Pedro Street
    {120, {92, 127}},  // Grand / LATTC
    {121, {57, 128}},  // Pico
    {122, {56, 129}},  // 7th Street / Metro Center
    {123, {58, 91}},   // LATTC / Ortho Institute
    {124, {59, 90}},   // Jefferson / USC
    {125, {60, 89}},   // Expo Park / USC
    {126, {61, 88}},   // Expo / Vermont
    {127, {62, 87}},   // Expo / Western
    {128, {63, 86}},   // Expo / Crenshaw
    {129, {64, 85}},   // Farmdale
    {130, {65, 84}},   // Expo / La Brea
    {131, {66, 83}},   // La Cienega / Jefferson
    {132, {67, 82}},   // Culver City
    {133, {68, 81}},   // Palms
    {134, {69, 80}},   // Westwood / Rancho Park
    {135, {70, 79}},   // Expo / Sepulveda
    {136, {71, 78}},   // Expo / Bundy
    {137, {72, 77}},   // 26th Street / Bergamot
    {138, {73, 76}},   // 17th Street / SMC
    {139, {74, 75}},   // Downtown Santa Monica
    {153, {109, 255}}, // 1st Street
    {154, {108, 255}}, // 5th Street Station
    {401, {0, 139}},   // Atlantic Station
    {402, {1, 138}},   // East LA Civic Center
    {403, {2, 137}},   // Maravilla
    {404, {3, 136}},   // Indiana
    {405, {4, 135}},   // Soto
    {406, {5, 134}},   // Mariachi Plaza
    {407, {6, 133}},   // Pico / Aliso
    {409, {52, 7}},    // Union Station
    {410, {51, 8}},    // Chinatown
    {411, {50, 9}},    // Lincoln / Cypress
    {412, {49, 10}},   // Heritage Square
    {413, {48, 11}},   // Southwest Museum
    {414, {47, 12}},   // Highland Park
    {415, {46, 13}},   // South Pasadena
    {416, {45, 14}},   // Fillmore
    {417, {44, 15}},   // Del Mar
    {418, {43, 16}},   // Memorial Park
    {419, {42, 17}},   // Lake
    {420, {41, 18}},   // Allen
    {421, {40, 19}},   // Sierra Madre Villa
    {422, {39, 20}},   // Arcadia
    {423, {38, 21}},   // Monrovia
    {424, {37, 22}},   // Duarte / City of Hope
    {425, {36, 23}},   // Irwindale
    {426, {35, 24}},   // Azusa Downtown
    {427, {34, 25}},   // APU / Citrus College Station
    {1401, {55, 130}}, // Grand Ave Arts / Bunker Hill
    {1402, {54, 131}}, // Historic Broadway
    {1403, {53, 132}}, // Little Tokyo / Arts District
    {1100, {33, 26}},  // Glendora
    {1101, {32, 27}},  // San Dimas Station
    {1102, {31, 28}},  // La Verne / Fairplex
    {1103, {30, 29}},  // Pomona North

};

static Station BD_Stops[] = {
    {201, {17, 16}}, // North Hollywood
    {202, {18, 15}}, // Universal / Studio City
    {203, {19, 14}}, // Hollywood / Highland
    {204, {20, 13}}, // Hollywood / Vine
    {205, {21, 12}}, // Hollywood / Western
    {206, {22, 11}}, // Vermont / Sunset
    {207, {23, 10}}, // Vermont / Santa Monica
    {208, {24, 9}},  // Vermont / Beverly Station
    {209, {43, 8}},  // Wilshire / Vermont Station
    {210, {44, 7}},  // Westlake / MacArthur Park Station
    {211, {45, 6}},  // 7th Street / Metro Center Station
    {212, {0, 5}},   // Pershing Square Station
    {213, {1, 4}},   // Civic Center / Grand Park
    {214, {2, 3}},   // Union Station
    {215, {25, 42}}, // Wilshire / Normandie
    {216, {26, 41}}, // Wilshire / Western
};

static Station CK_Stops[] = {
    {301, {31, 32}}, // Redondo Beach
    {302, {30, 33}}, // Douglas
    {303, {29, 34}}, // El Segundo
    {304, {28, 35}}, // Mariposa Station
    {305, {36, 9}},  // Aviation / Imperial
    {306, {37, 8}},  // Hawthorne / Lennox
    {307, {38, 7}},  // Crenshaw
    {308, {39, 6}},  // Vermont / Athens
    {309, {40, 5}},  // Harbor Freeway
    {310, {41, 4}},  // Avalon
    {311, {42, 3}},  // Willowbrook - Rosa Parks
    {312, {43, 2}},  // Lynwood
    {313, {44, 1}},  // Lakewood Blvd
    {314, {45, 0}},  // Norwalk Station
    {701, {27, 10}}, // Aviation / Century
    {702, {26, 11}}, // LAX / Metro Transit Center
    {703, {25, 12}}, // Westchester / Veterans
    {704, {24, 13}}, // Downtown Inglewood
    {705, {23, 14}}, // Fairview Heights
    {706, {22, 15}}, // Hyde Park
    {707, {21, 16}}, // Leimert Park
    {708, {20, 17}}, // Martin Luther King Jr
    {709, {19, 18}}, // Expo / Crenshaw K-Line

};

int get_all_lines(MLine **lines)
{
    MLine lines_c[] = {
        {AE_Stops, 801, ARRL(AE_Stops), AE_STRIP_ID, A_LED_VAL}, // A
        {BD_Stops, 802, ARRL(BD_Stops), BD_STRIP_ID, B_LED_VAL}, // B
        {CK_Stops, 803, ARRL(BD_Stops), CK_STRIP_ID, C_LED_VAL}, // C
        {BD_Stops, 805, ARRL(BD_Stops), BD_STRIP_ID, D_LED_VAL}, // D
        {AE_Stops, 804, ARRL(AE_Stops), AE_STRIP_ID, E_LED_VAL}, // E
        {CK_Stops, 807, ARRL(BD_Stops), CK_STRIP_ID, K_LED_VAL}, // K
    };
    *lines = malloc(sizeof(lines_c));
    memcpy(*lines, lines_c, sizeof(lines_c));
    return ARRL(lines_c);
}

int get_all_strips(LEDStrip **strips) {
    //     lgnd_strip = configure_led_strip(26, LGND_LEDS);
    // ae_strip = configure_led_strip(27, AE_LEDS);
    // bd_strip = configure_led_strip(14, BD_LEDS);
    // ck_strip = configure_led_strip(12, CK_LEDS);
    LEDStrip strips_c[] = {
        {NUM_LGND_LEDS, 26},
        {NUM_AE_LEDS, 27},
        {NUM_BD_LEDS, 14},
        {NUM_CK_LEDS, 12}
    };
    *strips = malloc(sizeof(strips_c));
    memcpy(*strips, strips_c, sizeof(strips_c));
    return ARRL(strips_c);
}