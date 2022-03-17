#include "frccert.h"

static const uint8_t Cert[] = {
    48,130,15,243,2,1,3,48,130,15,175,6,9,42,134,72,134,247,13,1,7,1,160,130,15,160,4,130,15,156,48,130,15,152,48,130,5,57,6,9,42,134,72,134,247,13,1,7,1,160,130,5,42,4,130,5,38,48,130,5,34,48,130,5,30,6,11,42,134,72,134,247,13,1,12,10,1,2,160,130,4,246,48,130,4,242,48,36,6,10,42,134,72,134,247,13,1,12,1,3,48,22,4,16,139,219,113,160,171,229,221,44,131,168,79,143,96,63,83,169,2,2,7,208,4,130,4,200,74,74,252,39,205,161,157,169,160,138,86,158,53,138,123,144,104,214,249,2,176,179,125,255,252,47,33,240,155,143,198,56,22,172,145,236,175,156,183,208,41,3,215,13,25,77,101,83,55,66,215,189,86,122,164,128,183,44,49,246,25,207,56,43,44,218,17,65,125,222,250,133,59,153,193,240,82,200,178,172,43,99,240,82,7,96,130,20,89,146,82,105,131,221,194,13,1,170,164,70,142,151,171,22,119,58,62,67,76,254,75,115,103,202,154,60,75,105,96,85,236,234,23,255,54,106,231,89,232,74,144,82,195,30,233,209,108,246,254,56,228,208,94,132,132,118,157,47,197,77,39,81,13,174,187,127,38,90,76,29,217,48,49,113,112,0,82,55,82,187,203,120,134,195,36,210,49,203,186,62,154,145,113,69,159,82,86,114,167,63,50,132,100,251,29,235,114,16,231,91,238,181,132,101,192,108,210,66,247,179,55,82,195,22,128,117,173,216,11,199,242,207,132,110,180,106,90,189,15,180,91,155,165,228,179,6,179,175,90,204,195,88,53,15,24,172,180,73,66,82,68,144,42,197,45,5,209,165,76,248,36,176,96,118,224,109,57,228,108,155,35,121,207,223,59,202,210,36,239,243,113,171,179,216,108,23,76,234,191,185,102,225,232,129,148,147,240,22,6,43,148,198,104,146,200,103,229,78,39,135,206,17,243,164,253,147,229,221,10,220,203,10,66,30,45,188,19,215,8,242,79,13,43,166,118,33,61,79,113,58,109,143,200,80,73,253,60,88,75,238,141,186,21,91,63,189,164,46,192,31,86,254,123,41,193,166,87,61,221,81,37,119,81,76,49,119,79,24,121,114,238,34,127,126,255,80,135,208,42,71,74,238,3,48,149,38,247,96,91,88,250,26,51,45,228,190,72,142,151,63,173,189,106,234,254,94,97,200,28,12,122,237,106,10,80,225,229,157,47,151,181,89,248,22,196,91,128,251,110,38,29,251,12,16,74,87,58,45,9,81,31,5,141,48,147,79,191,201,216,194,223,63,194,136,149,195,102,92,31,86,102,245,123,28,49,125,3,71,169,178,159,92,131,153,36,18,5,79,71,123,22,47,120,182,26,103,123,97,139,86,227,172,11,143,185,117,103,215,156,142,37,133,223,52,173,219,253,189,107,56,232,73,150,113,60,18,231,179,171,135,45,208,27,95,51,86,82,81,35,235,0,178,232,67,123,234,148,86,191,11,185,247,140,180,61,106,67,187,206,215,4,72,153,108,175,195,2,75,244,68,195,220,156,243,185,205,47,21,100,164,213,11,45,182,128,198,213,115,78,18,247,207,48,130,157,238,72,156,241,51,134,4,87,219,51,79,216,80,158,27,80,54,235,153,234,232,90,168,66,12,102,74,75,233,199,118,241,73,43,42,179,26,84,69,160,157,65,21,135,255,162,22,166,4,71,200,196,28,88,171,185,225,76,231,30,162,241,120,47,154,120,107,141,13,33,64,42,106,128,184,236,32,76,205,205,227,24,6,2,10,29,60,180,123,134,206,69,22,95,138,183,94,248,221,130,254,88,216,33,145,119,252,208,68,61,108,103,116,199,17,236,88,61,190,111,2,22,192,130,125,94,187,152,94,194,91,252,205,100,169,4,111,50,166,99,227,37,19,180,42,121,229,126,211,17,242,201,81,115,217,246,147,91,80,150,72,179,136,106,213,96,58,83,231,9,126,34,159,64,254,196,5,16,251,79,215,65,15,82,70,105,4,189,17,74,212,13,232,162,139,195,154,242,121,132,80,119,79,241,251,66,49,140,62,132,239,255,35,72,49,42,132,183,42,51,147,238,30,96,180,56,194,193,198,81,16,151,112,102,103,8,56,230,59,29,142,115,78,27,53,125,157,254,158,213,136,81,53,231,206,59,102,221,252,198,190,229,222,52,186,115,29,232,46,71,36,159,62,91,217,240,36,36,127,132,241,149,121,60,103,125,98,218,88,194,39,181,42,197,188,23,149,245,14,84,55,175,164,89,251,244,12,187,56,120,80,187,166,252,22,32,99,236,2,94,78,141,54,128,61,124,176,247,159,7,200,164,205,52,175,107,29,135,77,5,142,64,189,215,55,86,200,248,115,15,234,141,85,55,179,140,229,74,110,201,113,90,144,249,238,48,91,79,106,151,14,249,201,68,145,223,216,9,215,9,35,163,71,28,109,61,138,102,213,93,103,53,247,171,35,168,182,191,218,208,191,84,9,118,213,43,150,23,188,220,80,251,37,201,250,210,197,24,224,135,112,122,104,249,97,31,5,176,203,250,165,51,206,210,191,230,248,9,114,133,26,169,113,11,215,17,101,167,138,231,253,112,72,237,51,158,223,237,153,185,245,169,10,35,91,1,217,102,190,209,145,80,162,212,107,247,239,32,199,87,108,247,11,123,252,129,237,74,208,118,153,118,23,45,173,21,199,214,166,117,79,201,240,32,249,197,177,138,30,214,250,129,23,252,209,136,3,148,161,204,17,91,211,251,185,195,167,230,117,88,94,150,39,31,222,201,20,25,105,100,25,247,88,212,169,26,183,7,189,5,114,189,232,67,150,149,62,169,179,179,34,161,154,57,219,117,110,184,46,2,12,75,134,231,249,19,7,4,59,52,139,241,69,61,77,31,48,234,239,7,141,94,104,60,49,57,217,171,134,123,20,151,142,126,117,28,71,104,160,164,54,223,126,111,138,78,65,186,27,212,191,105,56,93,212,186,14,211,15,49,21,48,19,6,9,42,134,72,134,247,13,1,9,21,49,6,4,4,0,0,0,0,48,130,10,87,6,9,42,134,72,134,247,13,1,7,6,160,130,10,72,48,130,10,68,2,1,0,48,130,10,61,6,9,42,134,72,134,247,13,1,7,1,48,36,6,10,42,134,72,134,247,13,1,12,1,3,48,22,4,16,218,160,224,147,228,128,217,174,151,106,32,34,174,84,25,109,2,2,7,208,128,130,10,8,144,54,200,167,116,233,219,26,140,89,115,27,128,215,119,175,69,148,37,252,212,3,22,33,208,92,208,39,111,51,117,98,144,116,206,254,178,59,199,127,25,180,66,37,48,43,138,20,244,147,53,129,41,5,70,76,199,144,174,122,31,30,237,214,157,48,41,145,150,203,98,47,178,71,41,9,1,126,108,138,68,185,25,140,192,151,219,41,244,34,103,227,142,172,79,61,243,169,43,99,25,145,204,74,176,73,204,187,242,128,20,38,73,121,86,18,54,59,94,24,20,190,54,110,11,138,41,11,146,61,125,47,80,58,89,246,33,122,187,0,9,101,17,229,56,166,39,1,236,13,43,175,117,117,48,174,168,181,36,255,191,235,72,172,243,131,207,67,5,53,106,140,193,38,31,15,173,102,234,92,0,37,105,119,139,173,93,45,213,79,232,239,247,150,234,53,16,19,132,164,143,61,189,184,148,147,113,123,222,60,4,102,222,50,9,184,164,198,250,216,174,16,202,8,249,171,53,97,126,3,35,186,123,239,60,201,208,48,209,74,48,99,4,36,113,52,217,128,160,32,173,19,44,95,35,228,69,155,142,99,139,105,129,146,190,87,171,37,170,88,75,169,168,43,4,153,40,45,190,160,87,9,48,119,193,16,228,44,250,113,213,28,233,112,202,94,248,152,232,188,185,229,170,18,33,34,140,246,67,3,140,255,34,89,172,163,167,217,196,74,45,220,140,94,228,32,195,82,37,95,179,178,45,17,185,28,111,88,161,255,91,250,19,97,107,76,44,189,27,16,43,37,20,99,107,233,3,228,97,229,124,233,69,251,201,29,124,21,88,215,41,138,79,1,192,80,143,208,126,98,33,205,92,217,210,117,44,140,104,100,218,194,32,101,179,142,196,118,162,166,38,19,207,178,8,61,69,138,195,50,63,11,148,125,59,82,212,33,69,186,65,91,40,247,2,132,93,161,203,75,32,227,252,178,63,80,46,211,63,62,204,24,35,133,102,24,39,128,121,215,90,2,52,196,132,134,60,155,143,153,51,146,137,52,166,65,8,148,253,31,53,81,73,47,252,30,216,138,17,150,30,167,109,197,74,23,8,135,42,212,162,54,95,106,100,116,235,253,6,8,88,227,94,69,173,142,142,125,72,235,118,194,60,50,229,226,111,83,207,38,30,200,170,147,105,245,253,132,238,25,99,71,70,151,174,216,146,114,45,82,219,14,55,146,214,233,33,225,72,244,72,112,176,95,94,158,114,10,54,18,209,49,63,139,241,16,103,158,127,202,219,78,205,194,107,70,215,58,74,190,144,185,111,247,72,121,43,22,179,159,10,25,6,32,10,221,166,228,176,112,236,41,252,157,196,167,181,217,205,231,132,161,81,106,79,156,140,15,176,255,241,47,175,114,72,60,8,41,32,239,96,179,2,200,130,111,84,163,184,57,120,78,177,160,110,148,203,115,16,116,54,30,40,176,237,179,219,239,237,31,239,70,241,178,10,197,90,115,139,89,25,6,205,51,242,128,81,249,40,242,26,251,16,25,41,153,153,148,181,36,173,208,145,208,100,71,180,213,53,49,192,130,201,152,178,160,212,61,91,165,9,213,51,190,103,185,60,252,146,134,1,217,57,28,31,40,210,24,58,150,85,206,208,11,29,55,90,148,112,113,102,236,168,239,153,62,20,247,130,236,158,94,92,218,75,187,48,27,178,123,195,8,189,113,172,114,179,81,98,114,67,78,59,170,36,34,216,67,248,233,102,80,207,28,157,128,142,221,79,174,126,19,209,155,242,129,199,23,38,131,250,198,2,225,150,200,185,42,212,84,193,234,98,7,43,36,118,100,194,83,47,42,122,131,244,35,23,64,102,190,111,250,206,253,199,31,116,183,59,159,227,134,67,29,117,107,77,8,214,209,116,187,149,136,196,131,149,97,156,37,243,56,7,1,36,201,128,164,195,44,47,2,253,219,37,208,209,169,152,126,125,89,233,42,213,120,55,163,253,193,243,112,106,95,114,231,198,163,1,229,153,43,213,203,6,39,7,211,126,213,165,8,122,194,119,145,85,127,27,35,221,149,155,114,223,220,185,11,22,206,73,41,185,90,209,112,240,115,86,114,249,227,206,250,203,220,40,14,179,194,63,114,58,167,136,68,183,221,226,71,245,40,181,128,222,89,122,134,134,94,96,158,254,194,132,109,155,180,217,233,129,96,254,231,6,85,179,122,234,136,195,60,99,106,27,200,172,252,193,143,43,232,213,235,75,118,137,63,213,86,74,155,90,191,142,226,29,139,144,235,201,33,64,12,4,125,213,134,102,4,210,155,206,38,175,190,98,248,25,107,95,74,164,244,130,198,57,34,101,52,10,128,4,160,160,71,117,72,48,218,67,37,221,208,136,31,71,93,193,241,36,67,38,49,41,30,139,16,6,226,181,54,240,241,163,218,251,123,227,180,0,109,49,140,83,229,62,132,223,140,207,166,108,138,96,91,84,45,169,214,24,60,175,215,131,205,87,158,74,156,101,95,139,198,140,124,130,146,242,175,38,216,231,45,86,92,58,66,57,154,27,84,111,250,113,93,202,11,201,80,251,17,225,236,178,71,129,127,35,178,249,5,151,199,179,150,243,5,180,80,180,17,136,177,177,234,179,157,99,207,94,252,163,168,216,201,34,129,111,58,104,210,101,237,192,144,45,182,167,163,13,232,196,63,71,0,121,152,95,169,48,186,132,254,145,21,210,91,171,195,185,7,177,196,238,26,109,110,202,245,68,127,67,35,133,83,100,195,212,153,66,215,50,17,163,242,42,192,58,42,172,109,95,165,75,125,103,20,223,219,68,48,249,185,2,127,188,92,253,46,177,228,112,154,228,249,130,212,92,52,172,3,157,173,110,92,134,166,186,102,135,132,240,192,62,103,192,235,226,59,9,3,66,13,94,128,25,159,235,13,160,63,185,144,230,179,93,180,120,38,98,118,239,110,1,106,88,59,20,188,176,49,209,246,211,247,203,152,170,8,106,125,166,230,39,159,250,12,126,40,245,114,25,225,70,109,33,147,160,124,124,103,53,211,40,232,210,2,187,251,128,168,94,57,6,177,36,131,63,12,42,185,201,200,222,129,30,8,172,145,190,179,25,6,57,189,162,68,111,99,183,129,166,145,28,81,232,2,190,222,225,5,249,236,196,23,170,136,98,98,200,134,32,108,201,37,251,94,71,213,114,210,247,106,61,177,25,251,50,29,223,183,210,91,95,57,1,42,74,242,62,189,114,10,203,14,46,19,179,53,110,66,54,141,241,156,192,25,55,67,36,219,10,140,99,55,191,180,107,26,195,245,59,31,152,146,173,216,111,180,206,210,133,176,188,32,206,16,189,250,168,61,228,195,241,97,147,80,15,95,234,175,29,180,41,217,75,58,230,82,9,152,24,191,252,121,252,11,192,112,147,117,79,46,37,81,49,63,200,81,8,223,181,167,237,25,32,11,120,173,212,122,213,124,165,173,114,52,117,245,138,84,113,105,106,201,69,184,162,2,238,80,94,200,218,12,237,255,254,234,195,189,144,18,159,59,87,118,192,178,180,70,66,32,164,109,8,246,100,230,26,77,147,167,187,212,29,56,138,241,93,107,10,60,134,154,136,162,187,37,12,38,198,224,202,120,174,81,214,155,81,32,9,29,127,252,115,20,36,248,176,213,222,18,27,207,157,102,30,47,208,35,43,170,77,244,28,96,54,113,116,250,189,138,123,60,47,29,164,29,165,204,5,144,79,43,138,111,76,75,181,76,38,33,247,231,111,98,171,67,229,68,155,191,142,26,125,211,30,190,120,238,237,235,195,166,183,2,12,192,148,79,39,181,182,151,146,226,2,225,68,182,50,30,124,151,104,213,207,194,35,236,127,14,29,126,186,72,20,155,46,5,83,80,208,114,176,0,176,131,94,103,82,0,1,135,165,80,147,152,65,248,82,202,135,210,148,21,93,59,98,45,0,212,197,34,200,230,119,214,24,122,97,84,9,140,59,63,173,228,62,79,180,54,163,80,250,15,82,240,188,249,99,126,126,34,71,88,184,25,221,186,185,49,172,170,133,164,185,241,44,150,53,165,8,123,84,21,216,73,246,203,48,147,123,234,129,103,39,202,134,195,111,38,132,99,46,236,149,219,225,52,33,84,76,2,73,214,130,75,66,95,245,196,23,2,175,159,42,177,8,250,19,147,102,152,125,53,125,22,227,208,180,75,72,89,26,80,95,6,151,160,12,220,109,0,122,131,111,142,159,99,50,84,132,128,251,225,38,214,125,40,12,72,106,146,179,151,36,240,175,127,97,2,61,36,207,40,108,183,44,101,190,152,102,77,25,84,180,182,102,190,18,5,85,42,245,133,20,101,75,92,216,92,84,59,222,199,144,106,25,30,1,48,228,35,92,182,229,233,226,120,126,39,160,203,183,107,236,76,248,21,114,194,154,24,123,186,167,163,164,34,158,19,226,191,209,90,165,47,0,43,35,190,213,187,133,239,129,187,114,101,21,75,40,4,107,124,246,205,253,252,230,8,74,34,81,99,99,41,15,46,78,110,149,117,131,1,192,7,147,88,100,147,175,76,70,195,116,65,24,23,165,7,199,72,217,52,69,28,229,86,226,142,229,145,129,60,147,86,118,206,12,169,32,62,98,88,205,73,153,4,33,251,20,138,210,75,155,221,127,169,125,47,52,190,254,123,215,153,81,197,200,179,213,82,212,158,162,241,185,172,240,69,126,245,243,54,23,169,63,125,197,6,91,172,178,201,174,69,118,243,35,54,21,93,74,1,106,239,254,165,17,141,36,249,188,98,3,186,183,112,47,61,65,154,30,38,134,15,103,129,97,77,217,190,172,139,127,72,239,173,255,175,26,249,39,25,38,166,71,249,118,63,173,62,102,65,23,254,225,140,188,63,163,114,92,20,225,25,95,154,246,110,79,199,88,98,237,139,111,24,189,178,35,11,160,74,247,205,40,72,54,237,186,40,57,54,201,162,142,71,235,147,198,244,196,53,36,163,29,6,1,212,213,144,8,58,210,209,41,99,201,215,94,9,99,134,194,214,223,204,218,38,237,228,71,198,185,193,47,66,166,113,96,162,90,246,162,13,137,85,57,179,198,27,193,14,40,201,216,158,42,100,227,250,197,71,198,128,184,107,72,63,99,211,150,235,246,132,111,49,184,37,119,93,176,92,135,49,34,102,165,217,120,35,211,6,161,182,183,167,127,105,81,203,22,76,29,48,115,128,149,141,72,233,56,169,109,159,131,131,28,26,50,73,227,195,34,30,81,201,76,64,103,146,178,36,207,167,217,65,204,83,127,129,3,168,196,48,114,54,68,226,204,183,242,34,232,125,159,85,164,185,213,235,147,237,162,136,191,34,139,101,227,215,21,44,209,47,179,148,66,109,148,79,31,226,61,65,60,98,107,61,168,193,205,141,106,26,41,131,76,254,121,158,169,39,7,159,138,193,59,244,29,223,11,134,18,31,96,246,243,51,16,51,240,117,77,53,250,135,194,114,172,49,219,74,83,48,247,25,193,148,69,200,196,36,133,10,205,149,201,222,85,21,152,8,169,81,165,153,167,69,138,185,169,178,235,208,102,240,230,253,77,179,116,74,11,152,163,16,122,160,120,143,241,44,129,122,0,138,172,158,153,82,126,167,218,41,167,118,188,27,212,91,78,36,187,5,37,30,118,91,253,182,152,127,221,240,109,229,15,165,148,208,5,108,249,165,177,47,159,220,173,208,253,192,48,59,48,31,48,7,6,5,43,14,3,2,26,4,20,212,77,170,208,26,107,218,150,247,116,211,233,13,195,83,201,18,203,78,87,4,20,30,50,103,122,172,168,30,204,20,136,217,1,24,87,253,74,172,96,210,170,2,2,7,208
};

const uint8_t* QC_InternalGetCertificate(uint32_t* Length, const char** Password) {
    *Length = sizeof(Cert);
    *Password = "PLACEHOLDER";
    return Cert;
}

