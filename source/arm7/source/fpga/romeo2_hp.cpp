/*
	ROMEO2 ヘッドホンコネクタ ドライバ
*/
#include <nds.h>
#include "romeo2.h"
#include "romeo2_hp.h"

/****************************************************************************
	ヘッドホン端子に接続されているケーブルの種類を返す
****************************************************************************/
int hp_sense_connect(void)
{
	u16 port_val = HP_DETECT;

	if((port_val & HP_DETECT_HDSET)==0)
	{
		// AUDIO CABLE
		return (port_val & HP_DETECT_STDET) ? HP_MONO : HP_STEREO;
	}

	// VIDEO OUT CABLE (not suppported)
	if((port_val & HP_DETECT_STDET)==0) return HP_VIDEO;

	// no-connect
	return HP_NO_CONNECT;
}

/****************************************************************************
	ヘッドホン端子の、スイッチの状態を返す
****************************************************************************/
int hp_sense_switch(void)
{
	return (HP_DETECT & HP_DETECT_HSSW) ? HP_SWITCH_OFF : HP_SWITCH_ON;
}
