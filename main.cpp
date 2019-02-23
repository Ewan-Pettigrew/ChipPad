// ChipPad by Ewan Pettigrew - 16 Drum Pad triggers for "Chip Style" sounds.
// To do: Pulse width modulation. Expansion of tables. Programmable Sound Generators Mixed with noise for Percussion.


#include <stdio.h>
#include <nds.h>
#include <nds/ndstypes.h>
#include <time.h>
#include <chrono>

//the speed of the timer when using ClockDivider_1024
#define TIMER_SPEED (BUS_CLOCK/1024)

class DrumPad {
    int tickr, sound =0;
  public:
	int dpswitch =0, timetostop = 0;
	void play (char,int,int,int,int,int);
	void update_sound(int, int, int[][10]);
	void stop (void);
};

int main(void)
{
	soundEnable ();

	touchPosition touch;
	int x, y, xoffset, yoffset, toks, cubelength=40;
	DrumPad* drumPad= new DrumPad[16];

	videoSetMode(MODE_0_2D);
	videoSetModeSub(MODE_5_2D);

	//allocate vram banks
	vramSetBankA(VRAM_A_MAIN_BG);
	vramSetBankC(VRAM_C_SUB_BG);

	//create background
	int bgSub = bgInitSub(3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);

	//u16* videoMemoryMain = bgGetGfxPtr(bgMain);
	u16* videoMemorySub = bgGetGfxPtr(bgSub);

	PrintConsole topScreen;
	consoleInit(&topScreen, 3,BgType_Text4bpp, BgSize_T_256x256, 31, 0, true, true);
	consoleSelect(&topScreen);
	iprintf("Chip Pad");

	//Draw Drum Pads
	for (xoffset = 0; xoffset < ((cubelength+2)*4); xoffset = xoffset+ cubelength +2)
		for (yoffset = 0; yoffset < ((cubelength + 2) * 4); yoffset = yoffset + cubelength + 2)
			for (x = xoffset; x < xoffset + cubelength; x++)
				for (y = yoffset; y < yoffset + cubelength; y++)
				{
					videoMemorySub[x + y * 256] = ARGB16(1, 31, 0, 0);					
				}
	
	//array to store frequency,volume, pan tables *16 pads (pulse width is char so will require seperate pulse table)
	int patch[48][10] = {{1000,1500,2000,2500,3000,3500,3000,2500,2000,1500},
						{64,64,64,64,64,64,64,64,64,64},
						{64,64,64,64,64,64,64,64,64,64},
						{8000,7500,7000,6500,6000,5500,3000,2500,2000,1500},
						{64,64,64,64,64,64,64,64,64,64},
						{64,60,54,50,44,40,34,30,24,20},
						{1000,1500,2000,2500,3000,3500,3000,2500,2000,1500},
						{64,60,54,50,44,40,34,30,24,20},
						{64,64,64,64,64,64,64,64,64,64},
						{8000,5000,1000,5000,8000,5000,1000,5000,8000,4500},
						{64,64,64,64,64,64,64,64,64,64},
						{64,64,64,64,64,64,64,64,64,64},
						{1000,1500,1000,1500,1000,500,400,400,4000,400},
						{64,64,64,64,64,64,64,64,64,64},
						{64,64,64,64,64,64,64,64,64,64},
						{8000,500,8000,1000,8000,1500,8000,200,8000,2500},
						{64,64,64,64,64,64,64,64,64,64},
						{64,64,64,64,64,64,64,64,64,64},
						{400,410,420,430,440,450,460,470,480,490},
						{64,64,64,64,64,64,64,64,64,64},
						{64,64,64,64,64,64,64,64,64,64},
						{1000,500,2000,2500,3000,3500,3000,2500,2000,1500},
						{64,64,64,64,64,64,64,64,64,64},
						{64,64,64,64,64,64,64,64,64,64},
						{8000,7000,6000,5000,4000,3000,2000,1000,500,500},
						{01,05,10,20,25,30,40,45,50,64},
						{64,64,64,64,64,64,64,64,64,64},
						{8000,7000,6000,5000,4000,3000,2000,1000,500,500},
						{64,50,45,40,30,25,20,10,05,01},
						{64,64,64,64,64,64,64,64,64,64},
						{4000,5000,4000,5000,4000,5000,4000,5000,4000,5000},
						{64,64,64,64,64,64,64,64,64,64},
						{64,64,64,64,64,64,64,64,64,64},
						{1000,8000,1000,8000,1000,8000,1000,8000,1000,8000},
						{64,64,64,64,64,64,64,64,64,64},
						{64,64,64,64,64,64,64,64,64,64},
						{1000,8000,1000,8000,1000,8000,1000,8000,1000,8000},
						{64,50,45,40,30,25,20,10,05,01},
						{64,50,45,40,30,25,20,10,05,01},
						{500,500,500,2000,2000,2000,500,500,500,1500},
						{64,64,64,64,64,64,64,64,64,64},
						{64,64,64,64,64,64,64,64,64,64},
						{3000,1000,4000,2000,6000,4000,8000,6000,10000,8000},
						{64,64,64,64,64,64,64,64,64,64},
						{64,64,64,64,64,64,64,64,64,64},
						{6000,4000,8000,6000,10000,8000,4000,8000,6000,10000},
						{64,64,64,64,64,64,64,64,64,64},
						{64,64,64,64,64,64,64,64,64,64},
						};
		

	timerStart(0, ClockDivider_1024, 0, NULL);
	while (1)
	{
		for (int soundframe=1; soundframe<=10; soundframe++){// start the soundframe loop
			swiWaitForVBlank();
			touchRead(&touch);
			toks += timerElapsed(0);

			// these ifs have not been converted to a function yet as the plan is to decide whether to use graphics or draw pads then plan
			if (touch.px > 0 && touch.px <= cubelength && touch.py > 0 && touch.py <= cubelength && drumPad[0].dpswitch ==0) {drumPad[0].play(DutyCycle_25, 1, 64,64,5000,toks);}
			if (touch.px >= (cubelength + 2) && touch.px <= ((2 * cubelength) + 2) && touch.py > 0 && touch.py <= cubelength && drumPad[1].dpswitch ==0) {drumPad[1].play(DutyCycle_25, 1, 64,64,5000,toks);}
			if (touch.px >= ((2 * cubelength) + 4) && touch.px <= ((3 * cubelength) + 4) && touch.py > 0 && touch.py <= cubelength && drumPad[2].dpswitch ==0) {drumPad[2].play(DutyCycle_25, 1, 64,64,5000,toks);}
			if (touch.px >= ((3 * cubelength) + 6) && touch.px <= ((4 * cubelength) + 6) && touch.py > 0 && touch.py <= cubelength && drumPad[3].dpswitch ==0) {drumPad[3].play(DutyCycle_25, 1, 64,64,5000,toks);}
			
			if (touch.px > 0 && touch.px <= cubelength && touch.py >= (cubelength + 2) && touch.py <= ((2 *cubelength + 2))&& drumPad[4].dpswitch ==0) {{drumPad[4].play(DutyCycle_25, 1, 64,64,5000,toks);}}
			if (touch.px >= (cubelength + 2) && touch.px <= ((2 * cubelength) + 2) && touch.py >= (cubelength + 2) && touch.py <= ((2 *cubelength + 2))&& drumPad[5].dpswitch ==0) {{drumPad[5].play(DutyCycle_25, 1, 64,64,5000,toks);}}
			if (touch.px >= ((2 * cubelength) + 4) && touch.px <= ((3 * cubelength) + 4) && touch.py >= (cubelength + 2) && touch.py <= ((2 *cubelength + 2))&& drumPad[6].dpswitch ==0) {drumPad[6].play(DutyCycle_25, 1, 64,64,5000,toks);}
			if (touch.px >= ((3 * cubelength) + 6) && touch.px <= ((4 * cubelength) + 6) && touch.py >= (cubelength + 2) && touch.py <= ((2 *cubelength + 2))&& drumPad[7].dpswitch ==0) {drumPad[7].play(DutyCycle_25, 1, 64,64,5000,toks);}
				
			if (touch.px > 0 && touch.px <= cubelength && touch.py >= ((2 * cubelength) + 4) && touch.py <= ((3 *cubelength + 4))&& drumPad[8].dpswitch ==0) {drumPad[8].play(DutyCycle_25, 1, 64,64,5000,toks);}
			if (touch.px >= (cubelength + 2) && touch.px <= ((2 * cubelength) + 2) && touch.py >= ((2 * cubelength) + 4) && touch.py <=((3 *cubelength + 4))&& drumPad[9].dpswitch ==0) {drumPad[9].play(DutyCycle_25, 1, 64,64,5000,toks);}
			if (touch.px >= ((2 * cubelength) + 4) && touch.px <= ((3 * cubelength) + 4) && touch.py >= ((2 * cubelength) + 4) && touch.py <= ((3 *cubelength + 4))&& drumPad[10].dpswitch ==0) {drumPad[10].play(DutyCycle_25, 1, 64,64,5000,toks);}
			//soundPlayNoise(2000, 64,64) to implement later
			if (touch.px >= ((3 * cubelength) + 6) && touch.px <= ((4 * cubelength) + 6) && touch.py >= ((2 * cubelength) + 4) && touch.py <= ((3 *cubelength + 4))&& drumPad[11].dpswitch ==0) {drumPad[11].play(DutyCycle_25, 1, 64,64,5000,toks);}
				
			if (touch.px > 0 && touch.px <= cubelength && touch.py >= ((3 * cubelength) + 6) && touch.py <= ((4 *cubelength + 6))&& drumPad[12].dpswitch ==0) {drumPad[12].play(DutyCycle_25, 1, 64,64,50,toks);}
			if (touch.px >= (cubelength + 2) && touch.px <= ((2 * cubelength) + 2) && touch.py >= ((3 * cubelength) + 6) && touch.py <= ((4 *cubelength + 6))&& drumPad[13].dpswitch ==0) {drumPad[13].play(DutyCycle_25, 1, 64,64,5000,toks);}
			if (touch.px >= ((2 * cubelength) + 4) && touch.px <= ((3 * cubelength) + 4) && touch.py >= ((3 * cubelength) + 6) && touch.py <= ((4 *cubelength + 6))&& drumPad[14].dpswitch ==0) {drumPad[14].play(DutyCycle_25, 1, 64,64,5000,toks);}
			if (touch.px >= ((3 * cubelength) + 6) && touch.px <= ((4 * cubelength) + 6) && touch.py >= ((3 * cubelength) + 6) && touch.py <= ((4 *cubelength + 6))&& drumPad[15].dpswitch ==0) {drumPad[15].play(DutyCycle_25, 1, 64,64,5000,toks);}
		
			for(int ii=0; ii<=15; ii++){
			if (toks>=(drumPad[ii].timetostop)) {drumPad[ii].stop();}
			if (soundframe == 10) {soundframe=1;} // we are processing through 10 programmed frames for each pad
			drumPad[ii].update_sound(ii, soundframe,patch);
			}
		}	
		
	}	
	delete[] drumPad;
}

void DrumPad::play(char playDC, int playFreq,int playVol,int playPan, int lengthsampl, int tickr) {
	//Setting the frequency here is redundant, but I may use this method later.
	sound = soundPlayPSG(DutyCycle_25, playFreq, playVol,playPan);
	timetostop = tickr + lengthsampl;
	dpswitch=1;
	}

void DrumPad::update_sound(int chan, int mult, int patcha[][10]){
	soundSetFreq (sound, (patcha[(chan*3)][mult]));
	soundSetVolume (sound, (patcha[(chan*3)+1][mult]));
	soundSetPan (sound, (patcha[(chan*3)+2][mult]));
	}

void DrumPad::stop() {
    soundKill(sound);
	dpswitch=0;
	sound=0;
	}