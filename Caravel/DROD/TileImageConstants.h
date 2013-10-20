/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Deadly Rooms of Death.
 *
 * The Initial Developer of the Original Code is
 * Caravel Software.
 * Portions created by the Initial Developer are Copyright (C) 1995, 1996, 
 * 1997, 2000, 2001, 2002 Caravel Software. All Rights Reserved.
 *
 * Contributor(s):
 * Rik Cookney (timeracer), John Wm. Wicks (j_wicks)
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef TILEIMAGECONSTANTS_H
#define TILEIMAGECONSTANTS_H

#include <BackEndLib/Types.h>

#define TI_UNSPECIFIED  -1

//Tile image constants--ordering corresponds to order within bitmap containing
//all the tile images.
#define TI_SMAN_IYN		0
#define TI_SMAN_IYNE	1
#define TI_SMAN_IYE		2
#define TI_SMAN_IYSE	3
#define TI_SMAN_IYS		4
#define TI_SMAN_IYSW	5
#define TI_SMAN_IYW		6
#define TI_SMAN_IYNW	7
#define TI_SNK_N		8
#define TI_SNK_E		9
#define TI_SNK_S		10
#define TI_SNK_W		11
#define TI_SNK_EW		12
#define TI_SNK_NS		13
#define TI_SNK_NW		14
#define TI_SNK_NE		15
//********************************************************************************
#define TI_SWORD_IYN	16
#define TI_SWORD_IYNE	17
#define TI_SWORD_IYE	18
#define TI_SWORD_IYSE	19
#define TI_SWORD_IYS	20
#define TI_SWORD_IYSW	21
#define TI_SWORD_IYW	22
#define TI_SWORD_IYNW	23
#define TI_SNKT_S		24
#define TI_SNKT_W		25
#define TI_SNKT_N		26
#define TI_SNKT_E		27
#define TI_TARBABY		28
#define TI_TAR_SEI		29
#define TI_SNK_SW		30
#define TI_SNK_SE		31
//********************************************************************************
#define TI_SMAN_YN		32
#define TI_SMAN_YNE		33
#define TI_SMAN_YE		34
#define TI_SMAN_YSE		35
#define TI_SMAN_YS		36
#define TI_SMAN_YSW		37
#define TI_SMAN_YW		38
#define TI_SMAN_YNW		39
#define TI_TAR_NWSEI	40
#define TI_DOOR_YO		41
#define TI_DOOR_Y		42
#define TI_DOOR_M		43
#define TI_SNK_AN		44
#define TI_SNK_AE		45
#define TI_SNK_AS		46
#define TI_SNK_AW		47
//********************************************************************************
#define TI_SWORD_YN		48
#define TI_SWORD_YNE	49
#define TI_SWORD_YE		50
#define TI_SWORD_YSE	51
#define TI_SWORD_YS		52
#define TI_SWORD_YSW	53
#define TI_SWORD_YW		54
#define TI_SWORD_YNW	55
#define TI_TAR_NESWI	56
#define TI_TRAPDOOR		57
#define TI_DOOR_C		58
#define TI_TAR_NSEW		59
#define TI_TAREYE_WO	60
#define TI_TAREYE_EO	61
#define TI_TAREYE_WC	62
#define TI_TAREYE_EC	63
//********************************************************************************
#define TI_NTHR_N		64
#define TI_NTHR_NE		65
#define TI_NTHR_E		66
#define TI_NTHR_SE		67
#define TI_NTHR_S		68
#define TI_NTHR_SW		69
#define TI_NTHR_W		70
#define TI_NTHR_NW		71
#define TI_EYE_N		72
#define TI_EYE_NE		73
#define TI_EYE_E		74
#define TI_EYE_SE		75
#define TI_EYE_S		76
#define TI_EYE_SW		77
#define TI_EYE_W		78
#define TI_EYE_NW		79
//********************************************************************************
#define TI_ROACH_AN		80
#define TI_ROACH_ANE	81
#define TI_ROACH_AE		82
#define TI_ROACH_ASE	83
#define TI_ROACH_AS		84
#define TI_ROACH_ASW	85
#define TI_ROACH_AW		86
#define TI_ROACH_ANW	87
#define TI_EYE_AN		88
#define TI_EYE_ANE		89
#define TI_EYE_AE		90
#define TI_EYE_ASE		91
#define TI_EYE_AS		92
#define TI_EYE_ASW		93
#define TI_EYE_AW		94
#define TI_EYE_ANW		95
//********************************************************************************
#define TI_ROACH_N		96
#define TI_ROACH_NE		97
#define TI_ROACH_E		98
#define TI_ROACH_SE		99
#define TI_ROACH_S		100
#define TI_ROACH_SW		101
#define TI_ROACH_W		102
#define TI_ROACH_NW		103
#define TI_WALL			104                                        
#define TI_WALL_S		105
#define TI_WALL_B		106                                       
#define TI_WALL_BS		107
#define TI_TAR_N		108
#define TI_SHADO_DNE	109
#define TI_SHADO_LNE	110
#define TI_SHADO_DNW	111
//********************************************************************************
#define TI_QROACH_AN	112
#define TI_QROACH_ANE	113
#define TI_QROACH_AE	114
#define TI_QROACH_ASE	115
#define TI_QROACH_AS	116
#define TI_QROACH_ASW	117
#define TI_QROACH_AW	118
#define TI_QROACH_ANW	119
#define TI_STAIRS_1		120
#define TI_STAIRS_2		121
#define TI_TAR_E		122
#define TI_EMPTY_L		123
#define TI_EMPTY_D		124
#define TI_SHADO_LSW	125
#define TI_SHADO_DSW	126
#define TI_SHADO_LNW	127
//********************************************************************************
#define TI_QROACH_N		128
#define TI_QROACH_NE	129
#define TI_QROACH_E		130
#define TI_QROACH_SE	131
#define TI_QROACH_S		132
#define TI_QROACH_SW	133
#define TI_QROACH_W		134
#define TI_QROACH_NW	135
#define TI_STAIRS_3		136
#define TI_OB_NW		137
#define TI_OB_NE		138
#define TI_OBSHADO_DSW	139
#define TI_SHADO_LN		140
#define TI_SHADO_DN		141
#define TI_SHADO_LW		142
#define TI_SHADO_DW		143
//********************************************************************************
#define TI_GOBLIN_AN	144
#define TI_GOBLIN_ANE	145
#define TI_GOBLIN_AE	146
#define TI_GOBLIN_ASE	147
#define TI_GOBLIN_AS	148
#define TI_GOBLIN_ASW	149
#define TI_GOBLIN_AW	150
#define TI_GOBLIN_ANW	151
#define TI_STAIRS_4		152
#define TI_OB_SW		153
#define TI_OB_SE		154
#define TI_OBSHADO_LW	155
#define TI_SHADO_DNWI	156
#define TI_SHADO_LNWI	157
#define TI_SHADO_LNESW	158
#define TI_SHADO_DNESW	159
//********************************************************************************
#define TI_GOBLIN_N		160
#define TI_GOBLIN_NE	161
#define TI_GOBLIN_E		162
#define TI_GOBLIN_SE	163
#define TI_GOBLIN_S		164
#define TI_GOBLIN_SW	165
#define TI_GOBLIN_W		166
#define TI_GOBLIN_NW	167
#define TI_STAIRS_5		168
#define TI_OBSHADO_DNE	169
#define TI_OBSHADO_LN	170
#define TI_OBSHADO_DNW	171
#define TI_PIT_DN		172
#define TI_PIT_DNE		173
#define TI_PIT_LN		174
#define TI_PIT_LNE		175
//********************************************************************************
#define TI_REGG_A4		176
#define TI_REGG_A3		177
#define TI_REGG_A2		178
#define TI_REGG_4		179
#define TI_REGG_3		180
#define TI_REGG_2		181
#define TI_TARBABY_A	182
#define TI_DOOR_R		183
#define TI_OBSHADO_LSW	184
#define TI_OBSHADO_LNE	185
#define TI_OBSHADO_DN	186
#define TI_OBSHADO_LNW	187
#define TI_PIT_DS		188
#define TI_PIT_DSE		189
#define TI_PIT_LS		190
#define TI_PIT_LSE		191
//********************************************************************************
#define TI_TAR_W		192
#define TI_SCROLL		193
#define TI_ORB_L		194
#define TI_ORB_D		195
#define TI_POTION_K		196
#define TI_POTION_I		197
#define TI_BRAIN		198
#define TI_BRAIN_A		199
#define TI_OBSHADO_DW	200
#define TI_NTHR_SS		201
#define TI_ORB_N		202
#define TI_TAR_S		203
#define TI_SPIKE_D		204
#define TI_SPIKE		205
#define TI_SPIKE_DSW	206
#define TI_SPIKE_DNE	207
//********************************************************************************
#define TI_WW_N			208
#define TI_WW_NE		209
#define TI_WW_E			210
#define TI_WW_SE		211
#define TI_WW_S			212
#define TI_WW_SW		213
#define TI_WW_W			214
#define TI_WW_NW		215
#define TI_ARROW_N		216
#define TI_ARROW_NE		217
#define TI_ARROW_E		218
#define TI_ARROW_SE		219
#define TI_ARROW_S		220
#define TI_ARROW_SW		221
#define TI_ARROW_W		222
#define TI_ARROW_NW		223
//********************************************************************************
#define TI_MIMIC_SWORD_N	224
#define TI_MIMIC_SWORD_NE	225
#define TI_MIMIC_SWORD_E	226
#define TI_MIMIC_SWORD_SE	227
#define TI_MIMIC_SWORD_S	228
#define TI_MIMIC_SWORD_SW	229
#define TI_MIMIC_SWORD_W	230
#define TI_MIMIC_SWORD_NW	231
#define TI_WW_AN		232
#define TI_WW_ANE		233
#define TI_WW_AE		234
#define TI_WW_ASE		235
#define TI_WW_AS		236
#define TI_WW_ASW		237
#define TI_WW_AW		238
#define TI_WW_ANW		239
//********************************************************************************
#define TI_MIMIC_N		240
#define TI_MIMIC_NE		241
#define TI_MIMIC_E		242
#define TI_MIMIC_SE		243
#define TI_MIMIC_S		244
#define TI_MIMIC_SW		245
#define TI_MIMIC_W		246
#define TI_MIMIC_NW		247
#define TI_TAR_NW		248
#define TI_TAR_NE		249
#define TI_TAR_SW		250
#define TI_TAR_SE		251
#define TI_TAR_SWI		252
#define TI_TAR_NEI		253
#define TI_TAR_NWI		254
#define TI_TEMPTY		255

//********************************************************************************
#define TI_CHECKPOINT	256
#define TI_TARBLOOD_1	257
#define TI_TARBLOOD_2	258
#define TI_DOOR_YN		259
#define TI_DOOR_YS		260
#define TI_BLOOD_1		261
#define TI_BLOOD_2		262
#define TI_DEBRIS_1		263
#define TI_DEBRIS_2		264
#define TI_REGG_1		   265
#define TI_REGG_A1		266
//!!reuse #				267
//!!reuse #				268
#define TI_SPIDER_AN	269
#define TI_SPIDER_ANE	270
#define TI_SPIDER_AE	271
#define TI_SPIDER_ASE	272
#define TI_SPIDER_AS	273
#define TI_SPIDER_ASW	274
#define TI_SPIDER_AW	275
#define TI_SPIDER_ANW	276
#define TI_SPIDER_N		277
#define TI_SPIDER_NE	278
#define TI_SPIDER_E		279
#define TI_SPIDER_SE	280
#define TI_SPIDER_S		281
#define TI_SPIDER_SW	282
#define TI_SPIDER_W		283
#define TI_SPIDER_NW	284
#define TI_DOOR_YSE		285
#define TI_DOOR_YSWE	286
#define TI_DOOR_YSW		287
#define TI_DOOR_YNSE	288
#define TI_DOOR_YNSWE	289
#define TI_DOOR_YNSW	290
#define TI_DOOR_YNE		291
#define TI_DOOR_YNWE	292
#define TI_DOOR_YNW		293
#define TI_DOOR_YWE		294
#define TI_DOOR_YNS		295
#define TI_DOOR_YW		296
#define TI_DOOR_YE		297

#define TI_DOOR_MSE		298
#define TI_DOOR_MSWE	299
#define TI_DOOR_MSW		300
#define TI_DOOR_MNSE	301
#define TI_DOOR_MNSWE	302
#define TI_DOOR_MNSW	303
#define TI_DOOR_MNE		304
#define TI_DOOR_MNWE	305
#define TI_DOOR_MNW		306
#define TI_DOOR_MWE		307
#define TI_DOOR_MNS		308
#define TI_DOOR_MW		309
#define TI_DOOR_ME		310
#define TI_DOOR_MN		311
#define TI_DOOR_MS		312

#define TI_DOOR_CSE		313
#define TI_DOOR_CSWE	314
#define TI_DOOR_CSW		315
#define TI_DOOR_CNSE	316
#define TI_DOOR_CNSWE	317
#define TI_DOOR_CNSW	318
#define TI_DOOR_CNE		319
#define TI_DOOR_CNWE	320
#define TI_DOOR_CNW		321
#define TI_DOOR_CWE		322
#define TI_DOOR_CNS		323
#define TI_DOOR_CW		324
#define TI_DOOR_CE		325
#define TI_DOOR_CN		326
#define TI_DOOR_CS		327

#define TI_DOOR_RSE		328
#define TI_DOOR_RSWE	329
#define TI_DOOR_RSW		330
#define TI_DOOR_RNSE	331
#define TI_DOOR_RNSWE	332
#define TI_DOOR_RNSW	333
#define TI_DOOR_RNE		334
#define TI_DOOR_RNWE	335
#define TI_DOOR_RNW		336
#define TI_DOOR_RWE		337
#define TI_DOOR_RNS		338
#define TI_DOOR_RW		339
#define TI_DOOR_RE		340
#define TI_DOOR_RN		341
#define TI_DOOR_RS		342

#define TI_SWIRLDOT_1	343
#define TI_SWIRLDOT_2	344
#define TI_SWIRLDOT_3	345
#define TI_SWIRLDOT_4	346
#define TI_SWIRLDOT_5	347

#define TI_CHECKPOINT_L	348

#define TI_TDOORFALL_1  349
#define TI_TDOORFALL_1B	350
#define TI_TDOORFALL_1C	351
#define TI_TDOORFALL_2  352
#define TI_TDOORFALL_2B 353
#define TI_TDOORFALL_2C 354
#define TI_TDOORFALL_3  355
#define TI_TDOORFALL_3B 356
#define TI_TDOORFALL_3C 357
#define TI_TDOORFALL_4  358
#define TI_TDOORFALL_4B 359
#define TI_TDOORFALL_4C 360

const UINT TI_COUNT = 361;

#endif //...#ifndef TILEIMAGECONSTANTS_H

// $Log: TileImageConstants.h,v $
// Revision 1.22  2003/08/13 21:17:39  mrimer
// Fixed tar mother to spawn one move later the first time, making it every 30th move.
// Extended the roach egg hatching cycle by one turn, from four turns to five.
//
// Revision 1.21  2003/07/22 19:38:46  mrimer
// Moved RAND to BackEndLib/Types.h
//
// Revision 1.20  2003/05/23 21:43:03  mrimer
// Changed #includes for files in BackEndLib and FrontEndLib (from "file.h" to <file.h>).
//
// Revision 1.19  2003/05/22 23:35:34  mrimer
// Modified to use the new FrontEndLib and BackEndLib.
//
// Revision 1.18  2003/04/06 03:54:01  schik
// Ported to SGI.
// All filenames in CFiles and elsewhere are now in Unicode if the platform supports it.
//
// Revision 1.17  2002/08/25 19:02:37  erikh2000
// Added new TIs for trapdoor falling animation.
//
// Revision 1.16  2002/07/25 21:52:34  erikh2000
// Fixed tile count constant.
//
// Revision 1.15  2002/07/25 18:54:15  mrimer
// Added TI_CHECKPOINT_L.
//
// Revision 1.14  2002/07/04 21:12:32  erikh2000
// Added new TIs for swirl dots.
//
// Revision 1.13  2002/05/14 19:06:16  mrimer
// Added monster animation.
//
// Revision 1.12  2002/04/25 22:35:47  erikh2000
// Added TI_* constants for new door tile images.
//
// Revision 1.11  2002/04/23 03:12:44  erikh2000
// Added new constants for tar blood, removed mimic cursor constants, and changed value of TI_CHECKPOINT.
//
// Revision 1.10  2002/04/12 05:17:34  erikh2000
// Added TI_* constants for new tile images in GeneralTiles.bmp.
//
// Revision 1.9  2002/04/11 10:16:10  erikh2000
// Added TI_UNSPECIFIED constant.
//
// Revision 1.8  2002/04/09 10:05:40  erikh2000
// Fixed revision log macro.
//
