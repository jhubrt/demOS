#ifndef COMMANDS_H
#define COMMANDS_H

enum ScreensCommand_
{
    BLZ_CMD_CATEGORY_MASK = 0xF0,
    BLZ_CMD_COMMAND_MASK  = 0xF,

    BLZ_CMD_CURSOR_CATEGORY = 0x0,
    BLZ_CMD_LEFT       = 0x1,
    BLZ_CMD_RIGHT      = 0x2,
    BLZ_CMD_UP         = 0x3,
    BLZ_CMD_DOWN       = 0x4,
    BLZ_CMD_SELECT     = 0xE,
    BLZ_CMD_CODE_BACK  = 0xF,

    BLZ_CMD_FUNC_CATEGORY = 0x10,
    BLZ_CMD_F1     = 0x10,
    BLZ_CMD_F2     = 0x11,
    BLZ_CMD_F3     = 0x12,
    BLZ_CMD_F4     = 0x13,
    BLZ_CMD_F5     = 0x14,
    BLZ_CMD_F6     = 0x15,
    BLZ_CMD_F7     = 0x16,
    BLZ_CMD_F8     = 0x17,
    BLZ_CMD_F9     = 0x18,
    BLZ_CMD_F10    = 0x19,

    BLZ_CMD_VOICE1_CATEGORY = 0x20,
    BLZ_CMD_VOICE1_1  = 0x20,
    BLZ_CMD_VOICE1_2  = 0x21,
    BLZ_CMD_VOICE1_3  = 0x22,
    BLZ_CMD_VOICE1_4  = 0x23,
    BLZ_CMD_VOICE1_5  = 0x24,
    BLZ_CMD_VOICE1_6  = 0x25,
    BLZ_CMD_VOICE1_7  = 0x26,
    BLZ_CMD_VOICE1_8  = 0x27,
    BLZ_CMD_VOICE1_9  = 0x28,
    BLZ_CMD_VOICE1_10 = 0x29,
    BLZ_CMD_VOICE1_11 = 0x2A,
    BLZ_CMD_VOICE1_12 = 0x2B,

    BLZ_CMD_VOICE2_CATEGORY = 0x30,
    BLZ_CMD_VOICE2_1  = 0x30,
    BLZ_CMD_VOICE2_2  = 0x31,
    BLZ_CMD_VOICE2_3  = 0x32,
    BLZ_CMD_VOICE2_4  = 0x33,
    BLZ_CMD_VOICE2_5  = 0x34,
    BLZ_CMD_VOICE2_6  = 0x35,
    BLZ_CMD_VOICE2_7  = 0x36,
    BLZ_CMD_VOICE2_8  = 0x37,
    BLZ_CMD_VOICE2_9  = 0x38,
    BLZ_CMD_VOICE2_10 = 0x39,
    BLZ_CMD_VOICE2_11 = 0x3A,

    BLZ_CMD_LINE1_CATEGORY = 0x40,
    BLZ_CMD_Q      = 0x40,
    BLZ_CMD_W      = 0x41,
    BLZ_CMD_E      = 0x42,
    BLZ_CMD_R      = 0x43,
    BLZ_CMD_T      = 0x44,
    BLZ_CMD_Y      = 0x45,
    BLZ_CMD_U      = 0x46,
    BLZ_CMD_I      = 0x47,
    BLZ_CMD_O      = 0x48,
    BLZ_CMD_P      = 0x49,
    BLZ_CMD_BRACKL = 0x4A,
    BLZ_CMD_BRACKR = 0x4B,

    BLZ_CMD_LINE2_CATEGORY = 0x50,
    BLZ_CMD_A      = 0x50,
    BLZ_CMD_S      = 0x51,
    BLZ_CMD_D      = 0x52,
    BLZ_CMD_F      = 0x53,
    BLZ_CMD_G      = 0x54,
    BLZ_CMD_H      = 0x55,
    BLZ_CMD_J      = 0x56,
    BLZ_CMD_K      = 0x57,
    BLZ_CMD_L      = 0x58,
    BLZ_CMD_SEMICOLON = 0x59,
    BLZ_CMD_QUOTE   = 0x5A,

    BLZ_CMD_LINE3_CATEGORY = 0x60,
    BLZ_CMD_ANTISLASH = 0x60,
    BLZ_CMD_Z      = 0x61,
    BLZ_CMD_X      = 0x62,
    BLZ_CMD_C      = 0x63,
    BLZ_CMD_V      = 0x64,
    BLZ_CMD_B      = 0x65,
    BLZ_CMD_N      = 0x66,
    BLZ_CMD_M      = 0x67,
    BLZ_CMD_COMMA  = 0x68,
    BLZ_CMD_DOT    = 0x69,
    BLZ_CMD_SLASH  = 0x6A,

    BLZ_CMD_SOUND_CATEGORY = 0x70,
    BLZ_CMD_BASS_PLUS    = 0x70,
    BLZ_CMD_BASS_MINUS   = 0x71,
    BLZ_CMD_TREBLE_PLUS  = 0x72,
    BLZ_CMD_TREBLE_MINUS = 0x73
};
typedef enum ScreensCommand_ ScreensCommand;

#endif
