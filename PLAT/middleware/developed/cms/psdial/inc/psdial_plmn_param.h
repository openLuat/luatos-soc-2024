#ifndef __PS_DIAL_PLMN_PARAM_H__
#define __PS_DIAL_PLMN_PARAM_H__
/******************************************************************************
 ******************************************************************************
 Copyright:      - 2024- Copyrights of AirM2M Ltd.
 File name:      - psdial_plmn_param.h
 Description:    - PLMN static configuation functions
 History:        - 09/02/2024, Originated by zjyan
 ******************************************************************************
******************************************************************************/
#include "commontypedef.h"

/******************************************************************************
 *****************************************************************************
 * MARCO
 *****************************************************************************
******************************************************************************/
// Notify: When support AutoApn function, need to define this marco.
// #define FEATURE_PER_PLMN_ENABLE

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr)                 (sizeof(arr) / sizeof((arr)[0]))
#endif

/******************************************************************************
 *****************************************************************************
 * ENUM
 *****************************************************************************
******************************************************************************/
/*
 * Description: Used to define the operator ID. Each ID represents a combination of APN + operator names.
 * Composition: OPER + country abbreviation name defined by ISO organization + operator name + serial code;
 * Note: This enumeration type can only be extended backwards and cannot insert new values in the middle.
 */
typedef enum {
    OPER_GR_COSMOTE_0001 = 1,
    OPER_GR_VODA_0002,
    OPER_GR_WIND_0003,
    OPER_NL_TELE2_0004,
    OPER_NL_VODA_0005,
    OPER_NL_KPN_0006,
    OPER_NL_TLFRT_0007,
    OPER_NL_TMO_0008,
    OPER_NL_ORANGE_0009,
    OPER_NL_AGMS_0010,
    OPER_BE_PROXIMUS_0011,
    OPER_BE_TELENET_0012,
    OPER_BE_ORANGE_0013,
    OPER_BE_BASE_0014,
    OPER_FR_ORANGE_0015,
    OPER_FR_SFR_0016,
    OPER_FR_SFRFEMTO_0017,
    OPER_FR_CONTACT_0018,
    OPER_FR_FREE_0019,
    OPER_FR_LEGOS_0020,
    OPER_FR_BYTEL_0021,
    OPER_ES_VODA_0022,
    OPER_ES_ORANGE_0023,
    OPER_ES_YOIGO_0024,
    OPER_ES_MOVISTAR_0025,
    OPER_ES_VODA_0026,
    OPER_HU_TELENOR_0027,
    OPER_HU_TELEKOM_0028,
    OPER_HU_VODA_0029,
    OPER_BA_HTERONET_0030,
    OPER_BA_MTEL_0031,
    OPER_BA_MOBILE_0032,
    OPER_HR_HT_0033,
    OPER_HR_TELE2_0034,
    OPER_HR_VIP_0035,
    OPER_RS_TELENOR_0036,
    OPER_RS_MTS_0037,
    OPER_RS_TMO_0038,
    OPER_RS_SRB_0039,
    OPER_IT_TIM_0040,
    OPER_IT_VODA_0041,
    OPER_IT_WIND_0042,
    OPER_IT_ITA_0043,
    OPER_RO_VODA_0044,
    OPER_RO_TELEKOM_0045,
    OPER_RO_DIGI_0046,
    OPER_RO_ORANGE_0047,
    OPER_CH_SWISSCOM_0048,
    OPER_CH_SUNRISE_0049,
    OPER_CH_SALT_0050,
    OPER_CZ_TMO_0051,
    OPER_CZ_O2_0052,
    OPER_CZ_VODA_0053,
    OPER_SK_ORANGE_0054,
    OPER_SK_TELEKOM_0055,
    OPER_SK_O2_0056,
    OPER_AT_A1_0057,
    OPER_AT_TMA_0058,
    OPER_AT_3_0059,
    OPER_AT_TELERING_0060,
    OPER_GB_UK01_0061,
    OPER_GB_O2_0062,
    OPER_GB_O2_0063,
    OPER_GB_VODA_0064,
    OPER_GB_3UK_0065,
    OPER_GB_TRUPHONE_0066,
    OPER_GB_EE_0067,
    OPER_GB_JT_0068,
    OPER_GB_SURE_0069,
    OPER_GB_MANX_0070,
    OPER_DK_TDC_0071,
    OPER_DK_TELENOR_0072,
    OPER_DK_3_0073,
    OPER_SE_3_0074,
    OPER_SE_SWE_0075,
    OPER_SE_TELENOR_0076,
    OPER_SE_TELE2_0077,
    OPER_SE_SPRING_0078,
    OPER_NO_TELENOR_0079,
    OPER_NO_NETCOM_0080,
    OPER_NO_TELE2_0081,
    OPER_NO_TELIA_0082,
    OPER_FI_DNA_0083,
    OPER_FI_ELISA_0084,
    OPER_FI_SONERA_0085,
    OPER_LT_OMT_0086,
    OPER_LT_BITE_0087,
    OPER_LT_TELE2_0088,
    OPER_LV_LMT_0089,
    OPER_LV_TELE2_0090,
    OPER_LV_BITE_0091,
    OPER_EE_TELIA_0092,
    OPER_EE_ELISA_0093,
    OPER_RU_MTS_0094,
    OPER_RU_MEGAFON_0095,
    OPER_RU_TELE2_0096,
    OPER_RU_BEELINE_0097,
    OPER_UA_VODA_0098,
    OPER_UA_BEELINE_0099,
    OPER_UA_KS_0100,
    OPER_UA_LIFE_0101,
    OPER_UA_TRIMOB_0102,
    OPER_BY_VELCOM_0103,
    OPER_BY_MTS_0104,
    OPER_BY_LIFE_0105,
    OPER_MD_ORANGE_0106,
    OPER_MD_MOLDCELL_0107,
    OPER_MD_UNITE_0108,
    OPER_PL_PLUS_0109,
    OPER_PL_TM_0110,
    OPER_PL_ORANGE_0111,
    OPER_PL_PLAY_0112,
    OPER_DE_TDG_0113,
    OPER_DE_VODA_0114,
    OPER_DE_O2_0115,
    OPER_PT_VODA_0116,
    OPER_PT_NOS_0117,
    OPER_PT_MEO_0118,
    OPER_LU_POST_0119,
    OPER_LU_TANGO_0120,
    OPER_LU_ORANGE_0121,
    OPER_IE_VODA_0122,
    OPER_IE_3_0123,
    OPER_IE_METEOR_0124,
    OPER_IE_3_0125,
    OPER_IS_SIMINN_0126,
    OPER_IS_VODA_0127,
    OPER_IS_NOVA_0128,
    OPER_MT_VODA_0129,
    OPER_CY_MTN_0130,
    OPER_AM_UCOM_0131,
    OPER_BG_A1_0132,
    OPER_BG_VIVACOM_0133,
    OPER_BG_TELENOR_0134,
    OPER_TR_TCELL_0135,
    OPER_TR_VODA_0136,
    OPER_TR_AVEA_0137,
    OPER_GL_TELE_0138,
    OPER_SI_A1_0139,
    OPER_SI_MOBITEL_0140,
    OPER_SI_T2_0141,
    OPER_SI_TELEMACH_0142,
    OPER_MK_TMO_0143,
    OPER_MK_VIP_0144,
    OPER_ME_TELEKOM_0145,
    OPER_CA_TELUS_0146,
    OPER_CA_EASTLINK_0147,
    OPER_CA_FIDO_0148,
    OPER_CA_FREEDOM_0149,
    OPER_CA_CANVT_0150,
    OPER_CA_BELL_0151,
    OPER_CA_ROGERS_0152,
    OPER_CA_SASKTEL_0153,
    OPER_PR_CLARO_0154,
    OPER_MX_TELCEL_0155,
    OPER_MX_ATT_0156,
    OPER_TC_DIGICEL_0157,
    OPER_JM_CLARO_0158,
    OPER_JM_FLOW_0159,
    OPER_FA_ORANGE_0160,
    OPER_FA_DIGICEL_0161,
    OPER_VG_DIGICEL_0162,
    OPER_BQ_DIGICEL_0163,
    OPER_AW_DIGICEL_0164,
    OPER_CU_CUBACEL_0165,
    OPER_DO_ALTICE_0166,
    OPER_DO_CLARO_0167,
    OPER_DO_VIVA_0168,
    OPER_HT_DIGICEL_0169,
    OPER_TT_TSTT_0170,
    OPER_TT_DIGICEL_0171,
    OPER_AZ_AZERCELL_0172,
    OPER_AZ_BAKCELL_0173,
    OPER_AZ_FONEX_0174,
    OPER_AZ_NAR_0175,
    OPER_KZ_BEELINE_0176,
    OPER_KZ_KCELL_0177,
    OPER_KZ_TELE2_0178,
    OPER_BT_MOBILE_0179,
    OPER_BT_TASHICEL_0180,
    OPER_IN_VODA_0181,
    OPER_IN_AIRTEL_0182,
    OPER_IN_IDEA_0183,
    OPER_IN_RELIANCE_0184,
    OPER_IN_MOBILE_0185,
    OPER_IN_CELLONE_0186,
    OPER_IN_AIRCEL_0187,
    OPER_IN_DOLPHIN_0188,
    OPER_IN_VODAFONE_0189,
    OPER_IN_AIRCEL_0190,
    OPER_IN_AIRCEL_0191,
    OPER_IN_VIDEOCON_0192,
    OPER_IN_JIO_0193,
    OPER_IN_TELENOR_0194,
    OPER_IN_ETISALAT_0195,
    OPER_PK_UFONE_0196,
    OPER_PK_ZONG_0197,
    OPER_PK_TELENOR_0198,
    OPER_AF_AWCC_0199,
    OPER_AF_ROSHAN_0200,
    OPER_AF_MTN_0201,
    OPER_AF_ETISALAT_0202,
    OPER_LK_MOBITEL_0203,
    OPER_LK_DIALOG_0204,
    OPER_LK_ETISALAT_0205,
    OPER_LK_AIRTEL_0206,
    OPER_LK_HUTCH_0207,
    OPER_MM_MPTGSM_0208,
    OPER_LB_ALFA_0209,
    OPER_LB_TOUCH_0210,
    OPER_JO_ZAIN_0211,
    OPER_JO_UMNIAH_0212,
    OPER_JO_ORANGE_0213,
    OPER_SY_MTN_0214,
    OPER_IQ_ASIACELL_0215,
    OPER_IQ_ZAIN_0216,
    OPER_IQ_IRAQNA_0217,
    OPER_IQ_KOREK_0218,
    OPER_KW_ZAIN_0219,
    OPER_KW_OOREDOO_0220,
    OPER_KW_VIVA_0221,
    OPER_SA_STC_0222,
    OPER_SA_MOBILY_0223,
    OPER_SA_ZAIN_0224,
    OPER_YE_SABAFON_0225,
    OPER_YE_MTN_0226,
    OPER_YE_Y_0227,
    OPER_OM_OMANTEL_0228,
    OPER_OM_OOREDOO_0229,
    OPER_AE_ETISALAT_0230,
    OPER_AE_DU_0231,
    OPER_IL_PARTNER_0232,
    OPER_IL_CELLCOM_0233,
    OPER_IL_PCL_0234,
    OPER_IL_JAWWAL_0235,
    OPER_IL_WM_0236,
    OPER_IL_HOT_0237,
    OPER_IL_GOLAN_0238,
    OPER_BH_BATELCO_0239,
    OPER_BH_ZAIN_0240,
    OPER_BH_VIVA_0241,
    OPER_QA_OOREDOO_0242,
    OPER_QA_VODA_0243,
    OPER_MN_UNTLMN_0244,
    OPER_MN_MOBICOM_0245,
    OPER_NP_NT_0246,
    OPER_NP_NCELL_0247,
    OPER_NP_ST_0248,
    OPER_IR_MTCE_0249,
    OPER_UZ_BEELINE_0250,
    OPER_UZ_UCELL_0251,
    OPER_UZ_UMS_0252,
    OPER_TJ_TCELL_0253,
    OPER_TJ_MEGAFON_0254,
    OPER_TJ_BABILON_0255,
    OPER_TJ_BEELINE_0256,
    OPER_TJ_INDIGO_0257,
    OPER_KG_BEELINE_0258,
    OPER_KG_MEGACOM_0259,
    OPER_KG_O_0260,
    OPER_TM_MTS_0261,
    OPER_TM_CELL_0262,
    OPER_JP_SOFTBANK_0263,
    OPER_KR_SKT_0264,
    OPER_KR_LG_0265,
    OPER_KR_KT_0266,
    OPER_VN_MOBIFONE_0267,
    OPER_VN_GPC_0268,
    OPER_VN_VIETTEL_0269,
    OPER_VN_VNMOBILE_0270,
    OPER_VN_GMOBILE_0271,
    OPER_HK_CSL_0272,
    OPER_HK_3_0273,
    OPER_HK_3_0274,
    OPER_HK_SMC_0275,
    OPER_HK_CSL_0276,
    OPER_HK_CMHK_0277,
    OPER_HK_SMC_0278,
    OPER_HK_CSL_0279,
    OPER_HK_CSL_0280,
    OPER_MO_SMC_0281,
    OPER_MO_CTM_0282,
    OPER_MO_3MACAU_0283,
    OPER_MO_CT_0284,
    OPER_KH_CELLCARD_0285,
    OPER_KH_QB_0286,
    OPER_KH_SMART_0287,
    OPER_KH_METFONE_0288,
    OPER_KH_SEATEL_0289,
    OPER_LA_GSM_0290,
    OPER_LA_ETLMNW_0291,
    OPER_LA_UNITEL_0292,
    OPER_LA_BEELINE_0293,
    OPER_CN_CMCC_0294,
    OPER_CN_UNICOM_0295,
    OPER_CN_TELECOM_0296,
    OPER_TW_FET_0297,
    OPER_TW_APT_0298,
    OPER_TW_KGT_0299,
    OPER_TW_TSTAR_0300,
    OPER_TW_CHUNGHWA_0301,
    OPER_TW_MOB_0302,
    OPER_TW_MOBILE_0303,
    OPER_TW_TAT_0304,
    OPER_BD_GRAMEEN_0305,
    OPER_MV_DHIRAAGU_0306,
    OPER_MV_OOREDOO_0307,
    OPER_MY_MAXIS_0308,
    OPER_MY_CELCOM_0309,
    OPER_MY_DIGI_0310,
    OPER_MY_MOBILE_0311,
    OPER_MY_CELCOM_0312,
    OPER_AU_TELSTRA_0313,
    OPER_AU_OPTUS_0314,
    OPER_AU_VODA_0315,
    OPER_AU_3TELSTRA_0316,
    OPER_ID_XL_0317,
    OPER_ID_3_0318,
    OPER_TL_TCEL_0319,
    OPER_TL_TT_0320,
    OPER_PH_GLOBE_0321,
    OPER_PH_SMART_0322,
    OPER_PH_SUN_0323,
    OPER_PH_CURE_0324,
    OPER_TH_3G_0325,
    OPER_TH_AIS_0326,
    OPER_TH_TRUE_0327,
    OPER_TH_DTAC_0328,
    OPER_TH_TOT3G_0329,
    OPER_SE_TELIA_0330,
    OPER_TH_TRUE_0331,
    OPER_SG_SINGTEL_0332,
    OPER_SG_M1_0333,
    OPER_SG_STARHUB_0334,
    OPER_BN_PCSB_0335,
    OPER_BN_DSTCOM_0336,
    OPER_NZ_VODA_0337,
    OPER_NZ_SPARK_0338,
    OPER_NZ_2DEGREES_0339,
    OPER_PG_BMOBILE_0340,
    OPER_PG_DIGICEL_0341,
    OPER_TO_UCALL_0342,
    OPER_TO_DIGICEL_0343,
    OPER_VU_SMILE_0344,
    OPER_TO_DIGICEL_0345,
    OPER_FJ_VODA_0346,
    OPER_AS_BLUESKY_0347,
    OPER_NC_MOBNCL_0348,
    OPER_PF_VINI_0349,
    OPER_CK_KOKANET_0350,
    OPER_FM_TELE_0351,
    OPER_MH_NTA_0352,
    OPER_PW_PALAUCEL_0353,
    OPER_EG_ORANGE_0354,
    OPER_EG_VODA_0355,
    OPER_EG_ETISALAT_0356,
    OPER_DZ_MOBILIS_0357,
    OPER_DZ_DJEZZY_0358,
    OPER_DZ_OOREDOO_0359,
    OPER_MA_ORANGE_0360,
    OPER_MA_IAM_0361,
    OPER_MA_INWI_0362,
    OPER_TN_ORANGE_0363,
    OPER_TN_TUNTEL_0364,
    OPER_TN_TUNISIAN_0365,
    OPER_LY_LIBYANA_0366,
    OPER_LY_MADAR_0367,
    OPER_GM_GAMCEL_0368,
    OPER_GM_AFRICELL_0369,
    OPER_GM_COMIUM_0370,
    OPER_GM_QC_0371,
    OPER_SN_ORANGE_0372,
    OPER_SN_SENTEL_0373,
    OPER_SN_EXPRESSO_0374,
    OPER_MR_MATTEL_0375,
    OPER_MR_EMR_0376,
    OPER_MR_MAURITEL_0377,
    OPER_ML_ORANGE_0378,
    OPER_GN_ORANGE_0379,
    OPER_GN_LAGUI_0380,
    OPER_GN_MTN_0381,
    OPER_GN_CELLCOM_0382,
    OPER_CI_MOOV_0383,
    OPER_CI_ORANGE_0384,
    OPER_CI_KOZ_0385,
    OPER_CI_MTN_0386,
    OPER_BF_ONATEL_0387,
    OPER_BF_ORANGE_0388,
    OPER_NE_ORANGE_0389,
    OPER_TG_CELL_0390,
    OPER_TG_ETISALAT_0391,
    OPER_BJ_ETISALAT_0392,
    OPER_BJ_MTN_0393,
    OPER_BJ_BBCOM_0394,
    OPER_MU_MYT_0395,
    OPER_LR_LONESTAR_0396,
    OPER_LR_NOVAFONE_0397,
    OPER_LR_ORANGE_0398,
    OPER_SL_AIRTEL_0399,
    OPER_SL_MILLICOM_0400,
    OPER_GH_MTN_0401,
    OPER_GH_VODA_0402,
    OPER_GH_ATL_0403,
    OPER_GH_ATL_0404,
    OPER_NG_AIRTEL_0405,
    OPER_NG_MTN_0406,
    OPER_NG_GLO_0407,
    OPER_NG_9MOBILE_0408,
    OPER_TD_AIRTEL_0409,
    OPER_CF_ETISALAT_0410,
    OPER_CF_TELECEL_0411,
    OPER_CF_ORANGE_0412,
    OPER_CM_ORANGE_0413,
    OPER_CV_MOVEL_0414,
    OPER_CV_UNITEL_0415,
    OPER_ST_CSTMOVEL_0416,
    OPER_GQ_GETESA_0417,
    OPER_GQ_MUNI_0418,
    OPER_GA_LIBERTIS_0419,
    OPER_GA_ETISALAT_0420,
    OPER_GA_AZUR_0421,
    OPER_GA_MTN_0422,
    OPER_CD_VODACOM_0423,
    OPER_CD_TIGO_0424,
    OPER_AO_UNITEL_0425,
    OPER_AO_AGOMV_0426,
    OPER_GW_MTN_0427,
    OPER_GW_ORANGE_0428,
    OPER_GW_GTM_0429,
    OPER_SC_SEY_0430,
    OPER_SD_ZAIN_0431,
    OPER_RW_MTN_0432,
    OPER_RW_RWTEL_0433,
    OPER_RW_TIGO_0434,
    OPER_ET_MTN_0435,
    OPER_SO_TELE_0436,
    OPER_SO_SOMAFONE_0437,
    OPER_SO_GOLIS_0438,
    OPER_SO_TELSOM_0439,
    OPER_DJ_EVATIS_0440,
    OPER_KE_SAFCOM_0441,
    OPER_KE_AIRTEL_0442,
    OPER_KE_TELKOM_0443,
    OPER_TZ_VODACOM_0444,
    OPER_UG_MTN_0445,
    OPER_UG_UTL_0446,
    OPER_UG_AUL_0447,
    OPER_UG_AIRTEL_0448,
    OPER_BI_ONATEL_0449,
    OPER_BI_SMART_0450,
    OPER_BI_BDITL_0451,
    OPER_MZ_MCEL_0452,
    OPER_MZ_MOVITEL_0453,
    OPER_MZ_VODACOM_0454,
    OPER_ZM_MTN_0455,
    OPER_ZM_ZAMTEL_0456,
    OPER_MG_ORANGE_0457,
    OPER_RE_ORANGE_0458,
    OPER_RE_ONLY_0459,
    OPER_ZW_TELECEL_0460,
    OPER_ZW_ECONET_0461,
    OPER_NA_MTCNAM_0462,
    OPER_NA_TNMOBILE_0463,
    OPER_MW_TNM_0464,
    OPER_LS_VODACOM_0465,
    OPER_BW_ORANGE_0466,
    OPER_SZ_SWAZIMTN_0467,
    OPER_KM_HURI_0468,
    OPER_ZA_VODACOM_0469,
    OPER_ZA_TELKOMSA_0470,
    OPER_ZA_CELL_0471,
    OPER_ZA_MTN_0472,
    OPER_BZ_BTL_0473,
    OPER_GT_CLAROGTM_0474,
    OPER_GT_TIGO_0475,
    OPER_GT_MOVISTAR_0476,
    OPER_SV_CLAROSLV_0477,
    OPER_SV_DIGICEL_0478,
    OPER_SV_TIGO_0479,
    OPER_SV_MOVISTAR_0480,
    OPER_HN_CLARO_0481,
    OPER_HN_TIGO_0482,
    OPER_HN_CLARONIC_0483,
    OPER_HN_MOVISTAR_0484,
    OPER_CR_ICE_0485,
    OPER_CR_CLARO_0486,
    OPER_CR_MOVISTAR_0487,
    OPER_PA_PANCW_0488,
    OPER_PA_CLARO_0489,
    OPER_PA_DIGICEL_0490,
    OPER_PA_MOVISTAR_0491,
    OPER_PE_MOVISTAR_0492,
    OPER_PE_CLARO_0493,
    OPER_PE_VTP_0494,
    OPER_PE_ENTEL_0495,
    OPER_AR_MOVISTAR_0496,
    OPER_IN_IDEA_0497,
    OPER_IN_DOLPHIN_0498,
    OPER_PK_JAZZ_0499,
    OPER_AR_TEFMVNO_0500,
    OPER_AR_TP_0501,
    OPER_BR_TIM_0502,
    OPER_BR_CLARO_0503,
    OPER_BR_VIVO_0504,
    OPER_BR_SCTL_0505,
    OPER_BR_OI_0506,
    OPER_BR_OI_0507,
    OPER_BR_ALGAR_0508,
    OPER_BR_NEXTEL_0509,
    OPER_CL_ENTEL_0510,
    OPER_CL_MOVISTAR_0511,
    OPER_CL_CLARO_0512,
    OPER_CL_MOVISTAR_0513,
    OPER_CL_WOM_0514,
    OPER_CO_CLARO_0515,
    OPER_CO_TIGO_0516,
    OPER_CO_MOVISTAR_0517,
    OPER_CO_AVANTEL_0518,
    OPER_CO_ETB_0519,
    OPER_VE_DIGITEL_0520,
    OPER_VE_MOVISTAR_0521,
    OPER_VE_MOVILNET_0522,
    OPER_BO_VIVA_0523,
    OPER_BO_EMOVIL_0524,
    OPER_BO_TIGO_0525,
    OPER_GY_GTT_0526,
    OPER_EC_MOVISTAR_0527,
    OPER_EC_CLARO_0528,
    OPER_EC_CNT_0529,
    OPER_PY_HPGYSA_0530,
    OPER_PY_CLARO_0531,
    OPER_PY_TIGO_0532,
    OPER_PY_PERSONAL_0533,
    OPER_SR_TELEG_0534,
    OPER_SR_DIGICEL_0535,
    OPER_UY_ANTEL_0536,
    OPER_CL_MOVISTAR_0537,
    OPER_UY_CLARO_0538,
    OPER_XX_ATT_0539,
    OPER_ID_INDOSAT_0540,
    OPER_ID_AXIS_0541,
    OPER_ID_TELKOMSEL_0542,

    OPER_MAX_NUM = 2047
}OperatorId;

/******************************************************************************
 *****************************************************************************
 * STRUCT
 *****************************************************************************
******************************************************************************/

typedef struct OperatorConfig_tag {
    UINT32   operId : 12;   // OperatorId
    UINT32   ipType : 4;    // CmiPsPdnType, PDN type
    UINT32   authType : 4;  // CmiSecurityProtocol, auth type
    UINT32   spare :12;

    CHAR     *pApn;         // apn for LTE attach procedure
    CHAR     *pUserId;
    CHAR     *pPwd;
    CHAR     *pShortName;   // short name of operator
    CHAR     *pLongName;    // full name of operator
}OperatorConfig;

typedef struct PlmnOperTable_tag {
    UINT32    lastMcc : 8;      /* last 8 bit of MCC */
    UINT32    mnc : 12;
    UINT32    operId : 12;      /* OperatorId, max support 2048 operators */
}PlmnOperTable;



/******************************************************************************
 ******************************************************************************
 * External global variable
 ******************************************************************************
******************************************************************************/


/******************************************************************************
 *****************************************************************************
 * Functions
 *****************************************************************************
******************************************************************************/
#ifdef FEATURE_PER_PLMN_ENABLE

OperatorId psDialGetOperId(UINT16 mcc, UINT16 mnc);
const OperatorConfig *psDialGetOperCfgByOperId(OperatorId operId);

#endif

#endif

