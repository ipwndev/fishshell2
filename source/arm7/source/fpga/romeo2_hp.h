#ifndef __ROMEO2_HP_H__
#define __ROMEO2_HP_H__

#define HP_NO_CONNECT 0x00
#define HP_MONO       0x01
#define HP_STEREO     0x03
#define HP_VIDEO      0x04

int hp_sense_connect(void);

#define HP_SWITCH_ON  1
#define HP_SWITCH_OFF 0
int hp_sense_switch(void);

#endif
