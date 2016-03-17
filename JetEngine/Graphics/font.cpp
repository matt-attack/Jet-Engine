#include "font.h"
#include "CRenderer.h"
#include "CVertexBuffer.h"
#include "CTexture.h"

/* array size information */
const int sdf_tex_width = 128;
const int sdf_tex_height = 128;
const int sdf_num_chars = 191;
/* 'unsigned char sdf_data[]' is defined last */

/*
The following array holds the spacing info for rendering.
Note that the final 3 values need sub-pixel accuracy, so
they are multiplied by a scaling factor.  Make sure to
divide by scale_factor before using the 'offset' and
'advance' values.

Here is the data order in the following array:
[0] Unicode character ID
[1] X position in this texture
[2] Y position in this texture
[3] Width of this glyph in the texture
[4] Height of this glyph in the texture
[5] X Offset * scale_factor  | Draw the glyph at X,Y offset
[6] Y Offset * scale_factor  | relative to the cursor, then
[7] X Advance * scale_factor | advance the cursor by this.
*/
const int sdf_spacing[] = {
	32, 443, 506, 4, 4, -1500, 1500, 12500,
	33, 501, 412, 10, 40, 2375, 38062, 14312,
	34, 326, 489, 14, 16, 2125, 38375, 17312,
	35, 152, 221, 27, 40, 687, 37250, 27687,
	36, 333, 185, 29, 50, 750, 42875, 30312,
	37, 340, 323, 45, 40, 1000, 37875, 45812,
	38, 457, 323, 35, 40, 1062, 37875, 36062,
	39, 505, 280, 7, 16, 1875, 37812, 10375,
	40, 13, 455, 14, 46, 1000, 37812, 14812,
	41, 55, 278, 14, 46, 312, 37812, 14812,
	42, 0, 367, 26, 27, 562, 38687, 27000,
	43, 0, 308, 26, 28, 1000, 29437, 27500,
	44, 216, 351, 10, 16, 1125, 7187, 12312,
	45, 496, 505, 16, 7, 625, 17187, 16437,
	46, 501, 452, 10, 10, 1625, 7625, 12062,
	47, 152, 261, 24, 45, -312, 37375, 23250,
	48, 467, 132, 29, 41, 1187, 38750, 30375,
	49, 176, 432, 14, 40, -625, 38062, 15250,
	50, 152, 306, 26, 41, 687, 38750, 27250,
	51, 114, 258, 25, 41, 1000, 38750, 26937,
	52, 228, 352, 29, 40, 1125, 38062, 29562,
	53, 391, 82, 27, 41, 1437, 38062, 28750,
	54, 190, 222, 28, 41, 1000, 37937, 29250,
	55, 114, 380, 25, 40, 1062, 38062, 25687,
	56, 364, 82, 27, 41, 1000, 38750, 28875,
	57, 190, 181, 28, 41, 625, 38750, 28437,
	58, 217, 479, 9, 32, 2062, 28937, 12000,
	59, 217, 437, 9, 39, 1812, 29000, 12625,
	60, 0, 217, 27, 29, 1000, 30750, 28687,
	61, 72, 40, 27, 17, 1687, 24500, 29937,
	62, 0, 188, 27, 29, 1312, 30875, 28687,
	63, 38, 237, 20, 41, 250, 38750, 21125,
	64, 298, 237, 42, 43, 1187, 38000, 44250,
	65, 422, 323, 35, 40, -1437, 37687, 31812,
	66, 228, 392, 29, 40, 2750, 37625, 31437,
	67, 439, 465, 31, 41, 1250, 38250, 31500,
	68, 440, 280, 33, 40, 2750, 37625, 36687,
	69, 114, 420, 25, 40, 2750, 37625, 27875,
	70, 76, 391, 23, 40, 2750, 37625, 25437,
	71, 374, 280, 33, 41, 1250, 38250, 35687,
	72, 448, 185, 31, 40, 2750, 37625, 36375,
	73, 255, 309, 8, 40, 2937, 37625, 13187,
	74, 176, 261, 14, 45, -2500, 38062, 13375,
	75, 411, 132, 30, 40, 2750, 37625, 30312,
	76, 76, 255, 24, 40, 2750, 37687, 24937,
	77, 470, 465, 39, 40, 2750, 37625, 44312,
	78, 473, 280, 32, 40, 2750, 37625, 37000,
	79, 298, 407, 38, 41, 1250, 38250, 39750,
	80, 252, 82, 28, 40, 2062, 38062, 28250,
	81, 114, 82, 38, 52, 1250, 38250, 39750,
	82, 464, 237, 31, 40, 2062, 37937, 32687,
	83, 228, 185, 29, 41, 750, 38750, 29937,
	84, 351, 132, 30, 40, -1312, 37687, 26812,
	85, 417, 185, 31, 40, 2312, 37625, 35125,
	86, 340, 280, 34, 40, -1625, 37687, 30312,
	87, 0, 0, 53, 40, -1500, 37687, 49125,
	88, 407, 280, 33, 40, -1312, 37625, 30250,
	89, 340, 472, 33, 40, -1500, 37687, 29187,
	90, 228, 432, 29, 40, 625, 37625, 28937,
	91, 497, 82, 15, 47, 2000, 37750, 16500,
	92, 76, 431, 22, 41, -250, 37312, 20562,
	93, 137, 460, 15, 47, 437, 37750, 16500,
	94, 263, 485, 28, 26, 125, 38562, 28250,
	95, 470, 505, 26, 7, 312, -2687, 26562,
	96, 411, 172, 14, 13, 562, 41562, 15687,
	97, 0, 277, 24, 31, 812, 28875, 26812,
	98, 445, 82, 27, 41, 2125, 38875, 29687,
	99, 0, 336, 23, 31, 750, 28875, 23687,
	100, 418, 82, 27, 41, 750, 38812, 29562,
	101, 38, 450, 26, 31, 750, 28812, 27000,
	102, 38, 196, 20, 41, -687, 39375, 16375,
	103, 228, 309, 27, 43, 187, 28875, 28937,
	104, 152, 347, 26, 41, 2062, 38875, 27187,
	105, 217, 395, 8, 40, 2687, 38062, 12500,
	106, 0, 425, 13, 52, -3375, 38062, 10875,
	107, 114, 299, 25, 41, 2062, 38875, 25437,
	108, 98, 431, 15, 41, 2062, 38875, 13937,
	109, 340, 237, 42, 31, 2000, 28875, 43250,
	110, 38, 481, 26, 31, 2000, 28875, 28937,
	111, 76, 472, 28, 31, 750, 28875, 29312,
	112, 190, 395, 27, 42, 2000, 28875, 29375,
	113, 190, 437, 27, 42, 812, 28875, 29500,
	114, 58, 237, 17, 31, 2125, 28562, 18250,
	115, 0, 394, 22, 31, 1000, 28875, 23250,
	116, 492, 323, 19, 37, -750, 34500, 17312,
	117, 0, 246, 25, 31, 1062, 28312, 26812,
	118, 152, 476, 29, 30, -1562, 28312, 25187,
	119, 422, 237, 42, 30, -1062, 28312, 39187,
	120, 38, 166, 28, 30, -750, 28312, 25812,
	121, 298, 132, 28, 43, -1500, 28375, 24937,
	122, 13, 425, 22, 30, 500, 28312, 22250,
	123, 38, 374, 17, 48, 312, 39125, 17875,
	124, 288, 238, 7, 52, 6500, 40125, 19562,
	125, 38, 326, 17, 48, 812, 39187, 18000,
	126, 422, 267, 28, 11, 1375, 21437, 30625,
	160, 439, 506, 4, 4, -1500, 1500, 12500,
	161, 288, 344, 10, 40, 2375, 31500, 14312,
	162, 76, 135, 24, 40, 187, 38062, 26875,
	163, 336, 82, 28, 40, 500, 38250, 27625,
	164, 38, 422, 29, 28, -187, 31625, 28062,
	165, 381, 132, 30, 40, -937, 37687, 27687,
	166, 288, 291, 7, 52, 6375, 40062, 19250,
	167, 76, 348, 22, 43, 1937, 40312, 24812,
	168, 382, 269, 17, 9, 1375, 37312, 18812,
	169, 298, 280, 42, 43, 1375, 40250, 45000,
	170, 58, 196, 17, 27, 1625, 40375, 19687,
	171, 15, 159, 23, 24, 937, 25250, 24437,
	172, 298, 489, 28, 17, 1125, 21625, 29937,
	173, 399, 269, 21, 7, 1125, 10937, 23062,
	174, 298, 323, 42, 43, 1375, 40250, 44562,
	175, 475, 405, 21, 7, 2812, 41250, 25937,
	176, 118, 40, 19, 19, 1250, 38625, 20687,
	177, 190, 479, 27, 33, 1375, 31000, 29375,
	178, 99, 391, 15, 26, 1750, 39437, 18625,
	179, 22, 394, 15, 26, 1750, 39437, 18500,
	180, 450, 267, 14, 13, 1625, 41562, 15625,
	182, 441, 132, 26, 46, -1187, 39625, 26125,
	183, 340, 268, 8, 8, 2187, 20000, 11500,
	184, 216, 367, 8, 14, 1250, 1937, 14562,
	185, 176, 388, 13, 25, 1500, 39125, 14687,
	186, 99, 40, 19, 22, 1687, 40375, 21937,
	187, 15, 135, 23, 24, 937, 25250, 24687,
	188, 298, 448, 38, 41, 1500, 39375, 39312,
	189, 475, 363, 37, 42, 1500, 39312, 38875,
	190, 298, 366, 38, 41, 1312, 39437, 39750,
	191, 94, 295, 20, 41, 312, 31500, 21125,
	192, 228, 132, 35, 53, -1437, 50687, 31812,
	193, 263, 132, 35, 53, -1437, 50750, 31812,
	194, 263, 185, 35, 53, -1437, 50750, 31812,
	195, 373, 363, 35, 49, -1437, 47250, 31812,
	196, 408, 363, 35, 49, -1437, 46437, 31812,
	197, 298, 185, 35, 52, -1437, 50187, 31812,
	198, 0, 40, 50, 42, 500, 39812, 51062,
	199, 373, 412, 31, 51, 1250, 38250, 31500,
	200, 263, 344, 25, 53, 2750, 50750, 27875,
	201, 263, 291, 25, 53, 2750, 50750, 27875,
	202, 263, 238, 25, 53, 2750, 50750, 27875,
	203, 326, 132, 25, 48, 2750, 46437, 27875,
	204, 0, 135, 15, 53, -4250, 50750, 13187,
	205, 496, 132, 15, 53, 2937, 50750, 13187,
	206, 76, 295, 18, 53, -2125, 50750, 13187,
	207, 38, 278, 17, 48, -1562, 46437, 13250,
	208, 385, 323, 37, 40, -812, 38062, 37125,
	209, 443, 363, 32, 49, 2750, 47250, 37000,
	210, 0, 82, 38, 53, 1250, 50687, 39750,
	211, 38, 82, 38, 53, 1250, 50750, 39750,
	212, 76, 82, 38, 53, 1250, 50750, 39750,
	213, 190, 82, 38, 50, 1250, 47250, 39750,
	214, 190, 132, 38, 49, 1250, 46437, 39750,
	215, 50, 40, 22, 24, 1312, 26625, 24250,
	216, 152, 82, 38, 51, 1250, 42875, 39750,
	217, 408, 412, 31, 53, 2312, 50687, 35125,
	218, 439, 412, 31, 53, 2312, 50750, 35125,
	219, 470, 412, 31, 53, 2312, 50750, 35125,
	220, 373, 463, 31, 49, 2312, 46437, 35125,
	221, 340, 363, 33, 53, -1500, 50750, 29187,
	222, 228, 472, 29, 40, 2062, 38062, 31500,
	223, 228, 226, 29, 41, 1375, 38750, 29812,
	224, 114, 134, 24, 44, 812, 41312, 26812,
	225, 152, 432, 24, 44, 812, 41375, 26812,
	226, 152, 388, 24, 44, 812, 41375, 26812,
	227, 76, 215, 24, 40, 812, 37875, 26812,
	228, 76, 175, 24, 40, 812, 37062, 26812,
	229, 228, 82, 24, 47, 812, 44187, 26812,
	230, 382, 237, 40, 32, 437, 29000, 40875,
	231, 114, 460, 23, 42, 750, 28875, 23687,
	232, 190, 263, 26, 44, 750, 41312, 27000,
	233, 190, 307, 26, 44, 750, 41375, 27000,
	234, 190, 351, 26, 44, 750, 41375, 27000,
	235, 114, 178, 26, 40, 750, 37062, 27000,
	236, 98, 348, 15, 43, -5375, 41312, 10500,
	237, 55, 326, 15, 43, 1812, 41375, 10500,
	238, 55, 374, 18, 43, -3312, 41375, 10500,
	239, 495, 237, 17, 39, -2812, 37062, 10500,
	240, 408, 465, 29, 43, 625, 40312, 29437,
	241, 114, 218, 26, 40, 2000, 37875, 28937,
	242, 263, 397, 28, 44, 750, 41312, 29312,
	243, 479, 185, 28, 44, 750, 41375, 29312,
	244, 263, 441, 28, 44, 750, 41375, 29312,
	245, 308, 82, 28, 40, 750, 37875, 29312,
	246, 280, 82, 28, 40, 750, 37062, 29312,
	247, 38, 135, 28, 31, 875, 30000, 29562,
	248, 228, 267, 28, 42, 750, 33937, 29312,
	249, 472, 82, 25, 44, 1062, 41312, 26812,
	250, 152, 177, 25, 44, 1062, 41312, 26812,
	251, 152, 133, 25, 44, 1062, 41312, 26812,
	252, 114, 340, 25, 40, 1062, 37000, 26812,
	253, 340, 416, 28, 56, -1500, 41375, 24937,
	254, 390, 185, 27, 52, 1500, 38437, 29375,
	255, 362, 185, 28, 51, -1500, 37062, 24937,
	0
};

/*const int sdf_spacing[] = {
32,486,505,4,4,-1500,1500,30687,
33,494,95,17,41,6875,38687,30687,
34,9,450,23,17,4250,38687,30687,
35,463,147,36,40,-2562,38125,30687,
36,482,352,30,50,-562,40250,30687,
37,110,391,33,39,-687,37187,30687,
38,428,147,35,42,-2375,39312,30687,
39,97,487,10,17,10625,38687,30687,
40,74,366,22,49,6875,40187,30687,
41,488,199,22,49,1312,40187,30687,
42,0,177,28,27,1500,39375,30687,
43,37,298,31,31,125,31187,30687,
44,9,467,17,20,3687,10687,30687,
45,391,188,21,11,4562,19812,30687,
46,96,379,14,13,7125,10625,30687,
47,444,44,30,45,-1500,38687,30687,
48,242,0,30,42,687,39312,30687,
49,74,187,28,41,-437,38687,30687,
50,381,44,33,41,-2250,39312,30687,
51,179,362,32,42,-2312,39312,30687,
52,151,0,31,41,-1062,38687,30687,
53,144,447,32,41,-1375,38687,30687,
54,302,0,30,42,875,39312,30687,
55,110,430,31,41,1187,38687,30687,
56,272,0,30,42,62,39375,30687,
57,212,0,30,42,-187,39375,30687,
58,0,376,17,30,6062,27937,30687,
59,0,287,20,37,2687,28000,30687,
60,0,86,30,30,687,30812,30687,
61,179,490,30,21,687,26125,30687,
62,0,116,30,30,687,30812,30687,
63,74,446,25,41,5687,39375,30687,
64,382,250,35,46,-1875,36250,30687,
65,315,44,33,41,-4375,38687,30687,
66,249,44,33,41,-2000,38750,30687,
67,110,263,31,42,1750,39312,30687,
68,214,443,33,41,-1750,38687,30687,
69,214,146,34,41,-812,38687,30687,
70,460,95,34,41,-375,38687,30687,
71,110,179,31,42,1000,39312,30687,
72,318,95,35,41,-1750,38750,30687,
73,214,402,33,41,-875,38687,30687,
74,214,361,33,41,-2000,38687,30687,
75,448,250,38,41,-2000,38687,30687,
76,74,146,28,41,437,38687,30687,
77,391,147,37,41,-2937,38687,30687,
78,353,95,35,41,-2125,38687,30687,
79,110,221,31,42,0,39312,30687,
80,282,44,33,41,-1125,38687,30687,
81,317,411,31,48,0,39312,30687,
82,110,138,32,41,-1312,38687,30687,
83,179,320,32,42,-1437,39312,30687,
84,110,471,31,41,2937,38687,30687,
85,426,95,34,41,-437,38687,30687,
86,110,97,32,41,3500,38687,30687,
87,354,147,37,41,-500,38687,30687,
88,416,301,41,41,-5062,38687,30687,
89,249,466,34,41,2062,38687,30687,
90,317,147,37,41,-2375,38687,30687,
91,74,97,25,49,4000,40250,30687,
92,497,301,14,45,6937,38687,30687,
93,486,250,25,49,1125,40250,30687,
94,144,488,31,17,-125,38687,30687,
95,457,342,34,8,-1500,-5750,30687,
96,9,487,16,13,7687,42312,30687,
97,37,95,31,33,-375,30062,30687,
98,179,447,31,43,-375,40250,30687,
99,37,458,29,33,2187,30062,30687,
100,283,296,34,43,125,40250,30687,
101,37,234,30,33,375,30062,30687,
102,332,0,29,42,4250,40250,30687,
103,249,199,33,43,-1062,30125,30687,
104,182,0,30,42,-62,40250,30687,
105,179,95,30,45,-1187,42937,30687,
106,317,199,31,56,-3125,42937,30687,
107,283,469,34,42,500,40250,30687,
108,37,128,24,42,4250,40250,30687,
109,74,298,34,32,-2375,30062,30687,
110,37,329,30,32,-62,30062,30687,
111,37,201,30,33,687,30062,30687,
112,283,339,34,43,-2437,30125,30687,
113,179,404,31,43,187,30187,30687,
114,37,361,30,32,2312,30062,30687,
115,37,425,29,33,687,30000,30687,
116,74,228,29,39,2562,37312,30687,
117,37,393,30,32,1125,29375,30687,
118,0,55,30,31,2750,29375,30687,
119,74,415,34,31,687,29375,30687,
120,74,267,36,31,-3437,29375,30687,
121,420,199,37,42,-3750,29375,30687,
122,37,170,32,31,-312,29375,30687,
123,482,454,30,51,1750,40250,30687,
124,0,430,9,55,11000,40500,30687,
125,482,403,30,51,-2312,40250,30687,
126,463,187,30,12,687,21687,30687,
160,482,505,4,4,-1500,1500,30687,
161,493,0,17,41,6875,38687,30687,
162,144,359,28,47,1062,37187,30687,
163,283,95,35,41,-2125,39375,30687,
164,0,324,27,27,3125,29125,30687,
165,382,199,38,41,-2250,38750,30687,
166,499,147,9,48,11000,37187,30687,
167,74,51,27,46,1062,39312,30687,
168,416,342,21,10,8812,40937,30687,
169,424,0,34,34,-1500,35312,30687,
170,0,256,24,31,3750,39375,30687,
171,0,230,29,26,437,27875,30687,
172,37,491,30,17,687,23937,30687,
173,354,188,21,11,4562,19812,30687,
174,390,0,34,34,-1500,35312,30687,
175,437,342,20,8,9000,40250,30687,
176,9,430,20,20,5500,39375,30687,
177,74,330,30,36,687,33500,30687,
178,0,351,23,25,4937,39312,30687,
179,74,487,23,25,5312,39375,30687,
180,388,132,20,13,12625,42312,30687,
181,283,427,34,42,-2875,29375,30687,
182,317,459,31,46,1000,38687,30687,
183,96,366,14,13,8625,23750,30687,
184,408,132,16,14,2187,1500,30687,
185,0,406,21,24,5687,38937,30687,
186,0,146,25,31,4000,39375,30687,
187,0,204,29,26,500,27875,30687,
188,317,359,31,52,-750,42812,30687,
189,382,403,32,52,-750,42812,30687,
190,317,307,31,52,-375,43250,30687,
191,37,54,25,41,687,38687,30687,
192,416,352,33,51,-4375,48812,30687,
193,214,95,35,51,-4375,48812,30687,
194,214,44,35,51,-4375,48812,30687,
195,74,0,36,51,-4375,48812,30687,
196,179,44,35,51,-4375,48812,30687,
197,416,403,33,51,-4375,48812,30687,
198,457,301,40,41,-5250,38687,30687,
199,417,250,31,51,1750,39312,30687,
200,348,250,34,51,-812,48812,30687,
201,348,301,34,51,-812,48812,30687,
202,382,301,34,51,-812,48812,30687,
203,382,352,34,51,-812,48812,30687,
204,449,352,33,51,-875,48812,30687,
205,416,454,33,51,-875,48812,30687,
206,449,454,33,51,-875,48812,30687,
207,449,403,33,51,-875,48812,30687,
208,348,44,33,41,-1937,38687,30687,
209,144,44,35,51,-2125,48812,30687,
210,382,455,31,52,0,48750,30687,
211,348,352,31,52,0,48750,30687,
212,348,404,31,52,0,48750,30687,
213,348,456,31,52,0,48812,30687,
214,317,255,31,52,0,48750,30687,
215,214,484,28,28,1437,29875,30687,
216,110,0,41,44,-5062,40312,30687,
217,283,199,34,52,-437,48750,30687,
218,283,147,34,52,-437,48750,30687,
219,249,147,34,52,-437,48750,30687,
220,249,95,34,52,-437,48750,30687,
221,348,199,34,51,2062,48812,30687,
222,144,406,32,41,-1125,38687,30687,
223,214,187,32,43,-812,40312,30687,
224,249,286,31,45,-375,42250,30687,
225,283,251,33,45,-375,42250,30687,
226,249,331,31,45,-375,42250,30687,
227,214,317,31,44,-375,41125,30687,
228,214,273,31,44,-375,40937,30687,
229,457,199,31,50,-375,46750,30687,
230,458,0,35,33,-2312,30062,30687,
231,361,0,29,42,2187,30062,30687,
232,179,275,30,45,375,42250,30687,
233,283,382,32,45,375,42250,30687,
234,414,44,30,45,375,42250,30687,
235,144,139,30,44,375,40937,30687,
236,144,183,30,44,-1187,42312,30687,
237,249,242,32,44,-1187,42312,30687,
238,144,227,30,44,-1187,42312,30687,
239,110,305,30,43,-1187,40937,30687,
240,214,230,32,43,500,40250,30687,
241,110,348,30,43,-62,41187,30687,
242,179,185,30,45,687,42250,30687,
243,249,421,31,45,687,42250,30687,
244,474,44,30,45,687,42250,30687,
245,144,315,30,44,687,41125,30687,
246,144,95,30,44,687,40937,30687,
247,37,267,31,31,125,31312,30687,
248,388,95,38,37,-3562,32250,30687,
249,179,140,30,45,1125,42250,30687,
250,249,376,31,45,1125,42250,30687,
251,179,230,30,45,1125,42250,30687,
252,144,271,30,44,1125,40937,30687,
253,0,0,37,55,-3750,42312,30687,
254,110,44,34,53,-2437,40250,30687,
255,37,0,37,54,-3750,40937,30687,
0
};*/
/*const int sdf_spacing[] = {
32,252,233,4,4,-1500,1500,21062,
33,238,145,13,29,4250,27062,21062,
34,238,116,17,13,2437,27062,21062,
35,164,145,26,29,-2250,26687,21062,
36,142,87,22,35,-875,28125,21062,
37,194,58,24,28,-937,26000,21062,
38,164,174,25,30,-2125,27500,21062,
39,241,58,8,13,6812,27062,21062,
40,75,221,16,35,4250,28125,21062,
41,23,108,16,35,375,28125,21062,
42,189,235,20,20,562,27500,21062,
43,164,234,22,22,-375,21875,21062,
44,238,129,13,15,2062,7750,21062,
45,238,87,15,9,2625,14125,21062,
46,238,106,10,10,4437,7750,21062,
47,142,122,22,32,-1500,27062,21062,
48,97,155,22,30,0,27500,21062,
49,75,192,20,29,-812,27062,21062,
50,48,149,23,30,-2000,27500,21062,
51,119,212,23,30,-2062,27500,21062,
52,215,29,22,29,-1250,27062,21062,
53,119,182,23,30,-1437,27062,21062,
54,97,125,22,30,125,27500,21062,
55,169,29,23,29,312,27062,21062,
56,97,95,22,30,-437,27500,21062,
57,97,65,22,30,-562,27562,21062,
58,239,233,13,22,3687,19687,21062,
59,238,204,15,27,1375,19687,21062,
60,204,0,22,22,0,21625,21062,
61,48,239,22,15,0,18437,21062,
62,226,0,22,22,0,21625,21062,
63,23,143,18,30,3437,27500,21062,
64,23,29,25,33,-1750,25375,21062,
65,190,116,24,29,-3437,27062,21062,
66,214,145,24,29,-1875,27125,21062,
67,142,216,22,30,687,27500,21062,
68,190,145,24,29,-1687,27062,21062,
69,214,116,24,29,-1062,27062,21062,
70,190,87,24,29,-750,27000,21062,
71,142,186,22,30,187,27500,21062,
72,214,174,25,29,-1687,27062,21062,
73,214,87,24,29,-1062,27062,21062,
74,164,204,24,30,-1875,27062,21062,
75,119,29,27,29,-1812,27062,21062,
76,75,163,20,29,-187,27062,21062,
77,119,58,27,29,-2500,27062,21062,
78,213,204,25,29,-1937,27062,21062,
79,119,152,23,30,-500,27500,21062,
80,146,58,24,29,-1250,27062,21062,
81,119,87,23,34,-500,27500,21062,
82,218,58,23,29,-1375,27062,21062,
83,48,209,23,30,-1437,27500,21062,
84,146,29,23,29,1562,27062,21062,
85,189,174,25,30,-750,27062,21062,
86,192,29,23,29,1937,27062,21062,
87,164,116,26,29,-812,27062,21062,
88,23,0,29,29,-4000,27062,21062,
89,170,58,24,29,937,27062,21062,
90,164,87,26,29,-2125,27062,21062,
91,75,65,18,35,2250,28125,21062,
92,0,39,10,32,4250,27062,21062,
93,75,100,18,35,250,28125,21062,
94,119,242,23,13,-562,27062,21062,
95,23,244,25,7,-1500,-3500,21062,
96,238,96,12,10,4812,29500,21062,
97,23,220,22,24,-750,21125,21062,
98,119,121,23,31,-750,28125,21062,
99,183,0,21,24,1062,21125,21062,
100,189,204,24,31,-375,28125,21062,
101,52,0,22,24,-250,21187,21062,
102,97,215,21,30,2437,28125,21062,
103,48,119,24,30,-1250,21187,21062,
104,97,185,22,30,-500,28125,21062,
105,142,154,22,32,-1312,30000,21062,
106,0,0,23,39,-2625,30000,21062,
107,48,59,24,30,-125,28125,21062,
108,239,174,17,30,2437,28125,21062,
109,23,62,25,23,-2125,21125,21062,
110,96,0,22,23,-500,21125,21062,
111,23,196,22,24,0,21125,21062,
112,48,89,24,30,-2187,21187,21062,
113,48,179,23,30,-375,21187,21062,
114,74,0,22,23,1125,21125,21062,
115,162,0,21,24,0,21125,21062,
116,75,135,21,28,1250,26125,21062,
117,118,0,22,23,250,20687,21062,
118,140,0,22,23,1437,20687,21062,
119,23,85,25,23,0,20687,21062,
120,213,233,26,23,-2812,20687,21062,
121,48,29,27,30,-3062,20687,21062,
122,23,173,23,23,-687,20687,21062,
123,75,29,22,36,687,28125,21062,
124,0,71,7,39,7062,28312,21062,
125,97,29,22,36,-2062,28125,21062,
126,142,246,22,10,0,15437,21062,
0
};*/
/*const float scale_factor = 1000.000000;
const int sdf_spacing[] = {
32,960,999,4,4,-1500,1500,96937,
33,976,484,47,121,25062,118875,96937,
34,655,974,64,47,16625,118875,96937,
35,757,605,106,119,-4750,117062,96937,
36,569,605,86,150,1562,123875,96937,
37,863,605,97,116,1062,114062,96937,
38,655,605,102,125,-4125,121062,96937,
39,719,974,24,47,36750,118875,96937,
40,960,852,62,147,25062,123687,96937,
41,91,726,62,147,7375,123687,96937,
42,0,468,81,78,8000,121000,96937,
43,653,0,90,90,3687,95250,96937,
44,968,363,46,55,14875,30375,96937,
45,964,242,57,26,17625,59312,96937,
46,960,692,35,32,25812,30375,96937,
47,478,387,88,136,-1500,118937,96937,
48,569,755,86,125,5562,121062,96937,
49,302,809,80,121,1875,118937,96937,
50,192,746,95,123,-3812,121000,96937,
51,192,496,94,125,-3937,121062,96937,
52,689,121,90,121,-125,118875,96937,
53,192,869,94,123,-1062,118937,96937,
54,390,644,88,125,6000,120812,96937,
55,598,121,91,121,7062,118875,96937,
56,390,519,88,125,3437,121062,96937,
57,390,394,88,125,2812,121000,96937,
58,960,605,46,87,22437,85062,96937,
59,959,724,57,110,11812,85062,96937,
60,0,294,87,87,5375,94000,96937,
61,0,546,87,58,5375,79125,96937,
62,0,381,87,87,5375,94000,96937,
63,91,873,70,123,21375,121000,96937,
64,91,121,101,138,-2625,111187,96937,
65,774,242,95,121,-10437,118875,96937,
66,677,363,98,121,-3125,119062,96937,
67,91,509,89,125,8750,121062,96937,
68,872,363,96,121,-2187,118875,96937,
69,677,484,100,121,750,118875,96937,
70,777,484,100,121,2187,118875,96937,
71,91,384,89,125,6500,121062,96937,
72,655,730,102,121,-2187,118875,96937,
73,678,242,96,121,562,118875,96937,
74,192,373,96,123,-3125,118937,96937,
75,302,121,113,121,-2937,118937,96937,
76,302,688,80,121,4750,118937,96937,
77,569,242,109,121,-6000,118875,96937,
78,757,724,104,121,-3500,118937,96937,
79,91,259,91,125,3250,121062,96937,
80,775,363,97,121,-250,118875,96937,
81,478,242,91,145,3250,121062,96937,
82,869,242,95,121,-875,118875,96937,
83,192,621,94,125,-1187,121062,96937,
84,507,121,91,121,12625,118937,96937,
85,655,851,100,123,2000,118937,96937,
86,415,121,92,121,14437,118937,96937,
87,569,484,108,121,1687,118875,96937,
88,91,0,120,121,-12750,118937,96937,
89,877,484,99,121,9875,118937,96937,
90,569,363,108,121,-4125,118875,96937,
91,302,394,72,147,16000,123875,96937,
92,153,726,35,136,25187,118937,96937,
93,302,541,71,147,6812,123875,96937,
94,757,972,92,47,2937,118937,96937,
95,885,215,100,18,-1500,-21562,96937,
96,968,418,42,33,27625,130312,96937,
97,478,912,90,96,2187,91687,96937,
98,478,657,91,128,2187,123875,96937,
99,913,0,83,96,10250,91687,96937,
100,861,724,98,128,3687,123875,96937,
101,304,0,88,96,4437,91875,96937,
102,569,880,85,126,16625,123875,96937,
103,192,246,96,127,-125,92000,96937,
104,390,769,87,126,3125,123875,96937,
105,478,523,87,134,-437,132437,96937,
106,0,0,91,168,-6500,132437,96937,
107,861,852,99,126,4937,123875,96937,
108,0,168,67,126,16812,123875,96937,
109,885,121,101,94,-4125,91687,96937,
110,566,0,87,94,3125,91687,96937,
111,392,0,87,96,5375,91875,96937,
112,757,845,98,127,-4437,91687,96937,
113,478,785,91,127,3812,91875,96937,
114,302,930,88,94,10562,91687,96937,
115,830,0,83,96,5562,91687,96937,
116,390,895,83,117,11375,114562,96937,
117,479,0,87,94,6812,89625,96937,
118,743,0,87,92,12000,89625,96937,
119,91,634,101,92,5562,89562,96937,
120,779,121,106,92,-7500,89562,96937,
121,192,121,110,125,-8625,89625,96937,
122,211,0,93,92,2312,89562,96937,
123,302,242,88,152,8750,123875,96937,
124,0,604,21,165,37937,124562,96937,
125,390,242,88,152,-3937,123875,96937,
126,861,978,87,31,5375,65250,96937,
0
};*/



bool Font::Load(int texid, int texw, int texh, int charw, int charh)
{
	VertexElement elm9[] = { { ELEMENT_FLOAT2, USAGE_POSITION },
	{ ELEMENT_FLOAT2, USAGE_TEXCOORD } };
	this->buffer.SetVertexDeclaration(renderer->GetVertexDeclaration(elm9,2));

	//fill the ib

	short tmp[5000];
	int index = 0;
	for (int i = 0; i < 833; i++)
	{
		tmp[index++] = i * 4;
		tmp[index++] = i * 4 + 1;
		tmp[index++] = i * 4 + 2;
		tmp[index++] = i * 4 + 1;
		tmp[index++] = i * 4 + 2;
		tmp[index++] = i * 4 + 3;
	}
	this->ib.Data(tmp, 10000, 0);

	//should be enough data
	this->buffer.Data(tmp, 10000, 0);

	this->fonttexid = texid;

	for (int in = 0; in < 256; in++)
		this->spacing[in][0] = 0;

	int i = 0;
	while (sdf_spacing[i * 8] != 0)
	{
		memcpy(&this->spacing[sdf_spacing[i * 8]], &sdf_spacing[i * 8], 8 * sizeof(int));
		i++;
	}

	this->height = 43.0f;//28.0f;

	int border = 4;
	float resolution = 512.0f;
	for (int i = 0; i < sdf_num_chars; i++)
	{
		const int* p = &sdf_spacing[i * 8];

		float umin = ((float)p[1]) / resolution;
		float umax = ((float)(p[1] + p[3])) / resolution;
		float vmin = ((float)p[2]) / resolution;
		float vmax = ((float)(p[2] + p[4])) / resolution;

		Glyph* g = &glyphs[*p];
		g->u = umax;
		g->v = vmax;
		g->umin = umin;
		g->vmin = vmin;
	}

	this->height = this->spacing['O'][4];//'b'?

#ifdef _WIN32
	renderer->CreateShader(19, "Shaders/font.shdr");

	this->texture = resources.get<CTexture>("Oxygen-Regular.ttf_sdf.png");
	/*GLuint vertexShader = glCreateShader( GL_VERTEX_SHADER );
	glShaderSource( vertexShader, 1, &vertexSource, 0 );
	glCompileShader( vertexShader );

	GLuint fragmentShader = glCreateShader( GL_FRAGMENT_SHADER );
	glShaderSource( fragmentShader, 1, &fragmentSource, 0 );
	glCompileShader( fragmentShader );

	shader = glCreateProgram();
	glAttachShader( shader, vertexShader );
	glAttachShader( shader, fragmentShader );

	glLinkProgram( shader );

	glUseProgram(shader);*/
	//posAttrib = glGetAttribLocation( shader, "pos" );

	//texUnif = 0;//glGetUniformLocation( shader, "texture");
	//colorUnif = 0;//glGetUniformLocation( shader, "Color");

	//glGenBuffers( 1, &vb );

	int shaderCompiled = 1;
	//glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &shaderCompiled);//bad
	if (shaderCompiled == 0)
	{
		return false;
	}

	//GLint shaderCompiled;
	//glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &shaderCompiled);//bad
	if (shaderCompiled == 0)
	{
		return false;
	}

	return true;
#else
	//load shaders
	const char* fragmentSource ="precision mediump float;"
		"varying vec2 Texcoord;"
		"uniform sampler2D texture;"
		"uniform vec4 Color;"
		"void main(void) {"
		"vec4 col = texture2D(texture, Texcoord);"
		"gl_FragColor = vec4(Color.rgb,col.a);"//vec4(1,1,1,1.0 - col.r)*Color;"//"gl_FragColor = texture2D(texture, Texcoord);"//vec4(0,0,0,1-texture2D(texture, Texcoord).r);"//vec4(1,0,1,1);"//"gl_FragColor = texture2D(texture, Texcoord);"
		"}";

	const char* vertexSource =  "precision mediump float;"
		"attribute vec4 pos;\n"
		"varying vec2 Texcoord;"
		"void main() {"
		"Texcoord = pos.zw;"
		"gl_Position = vec4( pos.xy, 0, 1 );"
		"}";

	GLuint vertexShader = glCreateShader( GL_VERTEX_SHADER );
	glShaderSource( vertexShader, 1, &vertexSource, 0 );
	glCompileShader( vertexShader );

	GLuint fragmentShader = glCreateShader( GL_FRAGMENT_SHADER );
	glShaderSource( fragmentShader, 1, &fragmentSource, 0 );
	glCompileShader( fragmentShader );

	shader = glCreateProgram();
	glAttachShader( shader, vertexShader );
	glAttachShader( shader, fragmentShader );

	glLinkProgram( shader );

	glUseProgram(shader);
	posAttrib = glGetAttribLocation( shader, "pos" );

	texUnif = glGetUniformLocation( shader, "texture");
	colorUnif = glGetUniformLocation( shader, "Color");

	glGenBuffers( 1, &vb );

	GLint shaderCompiled;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &shaderCompiled);//bad
	if(shaderCompiled == GL_FALSE)
	{
		return false;
	}

	//GLint shaderCompiled;
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &shaderCompiled);//bad
	if(shaderCompiled == GL_FALSE)
	{
		return false;
	}

	return true;
#endif
};

#include "Shader.h"
#ifdef _WIN32
#undef DrawText
/* Here is the data order in the following array:
[0] Unicode character ID
[1] X position in this texture
[2] Y position in this texture
[3] Width of this glyph in the texture
[4] Height of this glyph in the texture
[5] X Offset * scale_factor  | Draw the glyph at X,Y offset
[6] Y Offset * scale_factor  | relative to the cursor, then
[7] X Advance * scale_factor | advance the cursor by this.*/

void Font::DrawText(const char* txt, float x, float y, float sx, float sy, unsigned int color)
{
	//if (showdebug > 2)
	//	return;
	auto shader = renderer->SetShader(19);
	//this probably doesnt need to be here
	renderer->SetCullmode(CULL_NONE);

	renderer->EnableAlphaBlending(true);
	float oldx = x;
	//glDepthFunc(GL_LEQUAL);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	auto old = renderer->current_texture;
	renderer->SetPixelTexture(0, this->texture);//"VeraMoBI.ttf_sdf.png"));//glyphsblack.png"));

	//renderer->context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	renderer->SetPrimitiveType(PT_TRIANGLELIST);
	//renderer->context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//fix color 
	shader->BindIL(&buffer.vd);

	//glBindTexture(GL_TEXTURE_2D, this->fonttexid);

	//glBindBuffer( GL_ARRAY_BUFFER, vb );

	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//renderer->d3ddev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);//point
	//renderer->d3ddev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);//point
	//renderer->d3ddev->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);//ANISOTROPIC);//point

	//glEnableVertexAttribArray(posAttrib);
	//glVertexAttribPointer(posAttrib, 4, GL_FLOAT, GL_FALSE, 4*4, 0);

	//glUniform1i(texUnif, 0);
	Vec4 colorv = Vec4(((float)((color & 0x00FF0000) >> 16)) / 255.0f, ((float)((color & 0x0000FF00) >> 8)) / 255.0f, ((float)(color & 0x000000FF)) / 255.0f, (float)(((color & 0xFF000000) >> 24)) / 255.0f);
	//glUniform4f(colorUnif, 1.0f, 1.0f, 1.0f, 1.0f);
	//if (shader == 0 || shader->cbuffers.find("Color") == shader->cbuffers.end())
	//	printf("EPIC FAIL\n");

	shader->cbuffers["Color"].UploadAndSet(&colorv, sizeof(Vec4));

	ib.Bind();

	//lets assume font size == 20px, we need a conversion factor
	float scale = sy / ((float)this->height);//28.0f;//80.0f/30.0f;//120.0f;//this is about correct

	//ok, lets 
	int len = strlen(txt);

	int max = 10000;
	float* data = (float*)alloca(max);

	int I = 0;
	for (int i = 0; i < len; i++)
	{
		int id = txt[i];
		if (id < 0)
			id = ((-id) ^ 0xFF) + 1;
		Glyph g = glyphs[id];

		if (txt[i] == '\n')
		{
			y += sy / (0.5f*renderer->yres);
			x = oldx;
			continue;
		}

		const int* p = spacing[id];
		if (id < 256 && p[0] != 0)
		{
			float x2 = x + (((float)p[5] / 1000.0f) / (0.5f*renderer->xres))*scale;// + g->bitmap_left * sx;
			float y2 = -y + 1.0f - sy / (0.5f*renderer->yres) + (((float)p[6] / 1000.0f) / (0.5f*renderer->yres))*scale;// + 1.0f;// - g->bitmap_top * sy;
			float w = ((float)p[3] / (0.5f*renderer->xres))*scale;///14.0f * sx;//this->width*sx;
			float h = ((float)p[4] / (0.5f*renderer->yres))*scale;///14.0f * sy;//this->height*sy;

			float umin = g.umin;//((float)p[1])/resolution;
			float umax = g.u;//((float)(p[1]+p[3]))/resolution;
			float vmin = g.vmin;//((float)p[2])/resolution;
			float vmax = g.v;//((float)(p[2]+p[4]))/resolution;
			/*float box[4][4] = {
				{x2,     y2    , umin, vmin},//0,0
				{x2 + w, y2    , umax, vmin},//1,0
				{x2,     y2 - h, umin, vmax},//0,1
				{x2 + w, y2 - h, umax, vmax},//1,1
				};*/

			data[I * 16] = x2;
			data[I * 16 + 1] = y2;
			data[I * 16 + 2] = umin;
			data[I * 16 + 3] = vmin;
			data[I * 16 + 4] = x2 + w;
			data[I * 16 + 5] = y2;
			data[I * 16 + 6] = umax;
			data[I * 16 + 7] = vmin;
			data[I * 16 + 8] = x2;
			data[I * 16 + 9] = y2 - h;
			data[I * 16 + 10] = umin;
			data[I * 16 + 11] = vmax;
			data[I * 16 + 12] = x2 + w;
			data[I * 16 + 13] = y2 - h;
			data[I * 16 + 14] = umax;
			data[I * 16 + 15] = vmax;
			I++;

			if ((I+1) * 16 * 4 > max)
			{
				//render now
				buffer.DataNoResize(data, max, 4 * 4);
				buffer.Bind();

				int numtris = max / 16;
				renderer->context->DrawIndexed(numtris * 6, 0, 0);

				I = 0;
			}
			//buffer.Data(&box, sizeof(float)*4*4, 0, 4*4);
			//buffer.Bind();

			//renderer->context->Draw(4, 0);

			x += (((float)p[7] / 1000.0f) / (0.5f*renderer->xres))*scale;///((20.0f/8.0f));//*((float)p[3]));//+= w;//width*sx;
			//y += 0;//this->height;
		}
	}

	buffer.DataNoResize(data, sizeof(float) * 4 * 4 * I, 4 * 4);
	buffer.Bind();

	ib.Bind();
	renderer->context->DrawIndexed(I * 6, 0, 0);

	//renderer->context->Draw(len * 4, 0);

	renderer->SetPixelTexture(0, old);
};
#else
void Font::DrawText(char* txt, float x, float y, float sx, float sy, unsigned int color)
{
	glUseProgram(shader);
	glEnable(GL_BLEND);

	glDepthFunc(GL_LEQUAL);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glBindTexture(GL_TEXTURE_2D, this->fonttexid);

	glBindBuffer(GL_ARRAY_BUFFER, vb );

	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 4, GL_FLOAT, GL_FALSE, 4*4, 0);

	glUniform1i(texUnif, 0);
	glUniform4f(colorUnif, ((float)((color & 0x00FF0000) >> 16))/255.0f, ((float)((color & 0x0000FF00) >> 8))/255.0f,((float)(color & 0x000000FF))/255.0f,(float)((color & 0xFF000000 >> 24))/255.0f);
	//glUniform4f(colorUnif, 1.0f, 1.0f, 1.0f, 1.0f);

	int len = strlen(txt);
	for (int i = 0; i < len; i++)
	{
		Glyph g = glyphs[(unsigned int)txt[i]];

		const int* p = &sdf_spacing[(txt[i]-32)*8];
		float scale = sy/28.0f;

		float x2 = x + (((float)p[5]/1000.0f)/(0.5f*renderer->xres))*scale;// + g->bitmap_left * sx;
		float y2 = -y + 1.0f - /*20.0f*/sy/(0.5f*renderer->yres) + (((float)p[6]/1000.0f)/(0.5f*renderer->yres))*scale;// + 1.0f;// - g->bitmap_top * sy;
		float w = ((float)p[3]/(0.5f*renderer->xres))*scale;///14.0f * sx;//this->width*sx;
		float h = ((float)p[4]/(0.5f*renderer->yres))*scale;///14.0f * sy;//this->height*sy;

		GLfloat box[4][4] = {
			{x2,     y2    , g.umin, g.vmin},//0,0
			{x2 + w, y2    , g.u, g.vmin},//1,0
			{x2,     y2 - h, g.umin, g.v},//0,1
			{x2 + w, y2 - h, g.u, g.v},//1,1
		};

		glBufferData(GL_ARRAY_BUFFER, sizeof box, box, GL_DYNAMIC_DRAW);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		x += (((float)p[7]/1000.0f)/(0.5f*renderer->xres))*scale;//x += this->width*sx;
		y += 0;//this->height;
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glDisable(GL_BLEND);
};
#endif

int Font::TextSize(const char* txt, int n)//fixme only returns width of first line, not widest
{
	if (txt == 0)
		return 0;

	float len = 0;
	float scale = 20.0f / ((float)this->height);//28.0f;
	for (int i = 0; txt[i] != 0; i++)
	{
		if (i == n && n != 0)
			break;

		if (txt[i] == '\n')//fixme to do multiple lines later
			break;

		int id = txt[i];
		if (id < 0)
			id = ((-id) ^ 0xFF) + 1;

		const int* p = spacing[id];
		if (*p != 0)
			len += ((float)p[7] / 1000.0f)*scale;
	}
	return len;//strlen(txt)*width;
}