#include <stdio.h>
#include <string.h>
#include "line_mapping.h"

#define NUM_LGND_LEDS 6
#define NUM_AE_LEDS 140
#define NUM_BD_LEDS 46
#define NUM_CK_LEDS 46

static Station AE_Stops[] = {
    {101, {255, 110}}, // Downtown Long Beach Station
    {102, {255, 111}}, // Pacific Ave Station
    {105, {112, 107}}, // Anaheim Street
    {106, {113, 106}}, // Pacific Coast Hwy
    {107, {114, 105}}, // Willow Street
    {108, {115, 104}}, // Wardlow Station
    {109, {116, 103}}, // Del Amo
    {110, {117, 102}}, // Artesia
    {111, {118, 101}}, // Compton
    {112, {119, 100}}, // Willowbrook - Rosa Parks Station
    {113, {120, 99}},  // 103rd Street / Watts Towers
    {114, {121, 98}},  // Firestone
    {115, {122, 97}},  // Florence
    {116, {123, 96}},  // Slauson
    {117, {124, 95}},  // Vernon
    {118, {125, 94}},  // Washington
    {119, {126, 93}},  // San Pedro Street
    {120, {127, 92}},  // Grand / LATTC
    {121, {128, 57}},  // Pico
    {122, {129, 56}},  // 7th Street / Metro Center
    {123, {91, 58}},   // LATTC / Ortho Institute
    {124, {90, 59}},   // Jefferson / USC
    {125, {89, 60}},   // Expo Park / USC
    {126, {88, 61}},   // Expo / Vermont
    {127, {87, 62}},   // Expo / Western
    {128, {86, 63}},   // Expo / Crenshaw
    {129, {85, 64}},   // Farmdale
    {130, {84, 65}},   // Expo / La Brea
    {131, {83, 66}},   // La Cienega / Jefferson
    {132, {82, 67}},   // Culver City
    {133, {81, 68}},   // Palms
    {134, {80, 69}},   // Westwood / Rancho Park
    {135, {79, 70}},   // Expo / Sepulveda
    {136, {78, 71}},   // Expo / Bundy
    {137, {77, 72}},   // 26th Street / Bergamot
    {138, {76, 73}},   // 17th Street / SMC
    {139, {75, 74}},   // Downtown Santa Monica
    {153, {255, 109}}, // 1st Street
    {154, {255, 108}}, // 5th Street Station
    {401, {139, 0}},   // Atlantic Station
    {402, {138, 1}},   // East LA Civic Center
    {403, {137, 2}},   // Maravilla
    {404, {136, 3}},   // Indiana
    {405, {135, 4}},   // Soto
    {406, {134, 5}},   // Mariachi Plaza
    {407, {133, 6}},   // Pico / Aliso
    {409, {7, 52}},    // Union Station
    {410, {8, 51}},    // Chinatown
    {411, {9, 50}},    // Lincoln / Cypress
    {412, {10, 49}},   // Heritage Square
    {413, {11, 48}},   // Southwest Museum
    {414, {12, 47}},   // Highland Park
    {415, {13, 46}},   // South Pasadena
    {416, {14, 45}},   // Fillmore
    {417, {15, 44}},   // Del Mar
    {418, {16, 43}},   // Memorial Park
    {419, {17, 42}},   // Lake
    {420, {18, 41}},   // Allen
    {421, {19, 40}},   // Sierra Madre Villa
    {422, {20, 39}},   // Arcadia
    {423, {21, 38}},   // Monrovia
    {424, {22, 37}},   // Duarte / City of Hope
    {425, {23, 36}},   // Irwindale
    {426, {24, 35}},   // Azusa Downtown
    {427, {25, 34}},   // APU / Citrus College Station
    {1401, {130, 55}}, // Grand Ave Arts / Bunker Hill
    {1402, {131, 54}}, // Historic Broadway
    {1403, {132, 53}}, // Little Tokyo / Arts District
    {1100, {26, 33}},  // Glendora
    {1101, {27, 32}},  // San Dimas Station
    {1102, {28, 31}},  // La Verne / Fairplex
    {1103, {29, 30}},  // Pomona North

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
    {301, {32, 31}}, // Redondo Beach
    {302, {33, 30}}, // Douglas
    {303, {34, 29}}, // El Segundo
    {304, {35, 28}}, // Mariposa Station
    {305, {9, 36}},  // Aviation / Imperial
    {306, {8, 37}},  // Hawthorne / Lennox
    {307, {7, 38}},  // Crenshaw
    {308, {6, 39}},  // Vermont / Athens
    {309, {5, 40}},  // Harbor Freeway
    {310, {4, 41}},  // Avalon
    {311, {3, 42}},  // Willowbrook - Rosa Parks
    {312, {2, 43}},  // Lynwood
    {313, {1, 44}},  // Lakewood Blvd
    {314, {0, 45}},  // Norwalk Station
    {701, {10, 27}}, // Aviation / Century
    {702, {11, 26}}, // LAX / Metro Transit Center
    {703, {12, 25}}, // Westchester / Veterans
    {704, {13, 24}}, // Downtown Inglewood
    {705, {14, 23}}, // Fairview Heights
    {706, {15, 22}}, // Hyde Park
    {707, {16, 21}}, // Leimert Park
    {708, {17, 20}}, // Martin Luther King Jr
    {709, {18, 19}}, // Expo / Crenshaw K-Line

};

int get_all_lines(MLine **lines)
{
    MLine lines_c[] = {
        {AE_Stops, 801, ARRL(AE_Stops), AE_STRIP_ID, A_LED_VAL}, // A
        {BD_Stops, 802, ARRL(BD_Stops), BD_STRIP_ID, B_LED_VAL}, // B
        {CK_Stops, 803, ARRL(CK_Stops), CK_STRIP_ID, C_LED_VAL}, // C
        {BD_Stops, 805, ARRL(BD_Stops), BD_STRIP_ID, D_LED_VAL}, // D
        {AE_Stops, 804, ARRL(AE_Stops), AE_STRIP_ID, E_LED_VAL}, // E
        {CK_Stops, 807, ARRL(CK_Stops), CK_STRIP_ID, K_LED_VAL}, // K
    };
    *lines = malloc(sizeof(lines_c));
    memcpy(*lines, lines_c, sizeof(lines_c));
    return ARRL(lines_c);
}

int get_all_strips(LEDStrip **strips)
{
    //     lgnd_strip = configure_led_strip(26, LGND_LEDS);
    // ae_strip = configure_led_strip(27, AE_LEDS);
    // bd_strip = configure_led_strip(14, BD_LEDS);
    // ck_strip = configure_led_strip(12, CK_LEDS);
    LEDStrip strips_c[] = {
        {NUM_AE_LEDS, 27},
        {NUM_BD_LEDS, 14},
        {NUM_CK_LEDS, 12}};
    *strips = malloc(sizeof(strips_c));
    memcpy(*strips, strips_c, sizeof(strips_c));
    return ARRL(strips_c);
}

LEDStrip get_lgnd_strip()
{
    LEDStrip s = {NUM_LGND_LEDS, 26};
    return s;
}