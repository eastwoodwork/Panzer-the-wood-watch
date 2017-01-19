/**
  ****************************************************************************** 
  * @author   eastwood.work
  * @version  /
  * @date     /
  * @brief    Mapping the LED matrix to arrays. The bits of each element of the
							array stand for ON/OFF of the corresponding pixels.
  */ 
/* Includes ------------------------------------------------------------------*/
#include "display2_mapping.h"

bool DSP_LAYER1_EN = FALSE,
		 DSP_LAYER2_EN = FALSE,
		 DSP_LAYER3_EN = FALSE;
		 
/*Bit-Matrix to mapping the LED Matrix pixels*/
unsigned char BG[MATRIX_X_LEN]={0};//{0x03,0x07,0x0f,0x1f,0x3f,0x7f,0xff};
unsigned char LAYER_1[MATRIX_X_LEN]={0},LAYER_2[MATRIX_X_LEN]={0},LAYER_3[MATRIX_X_LEN]={0};
/*Graphic compact code*/
unsigned char const Jet_CODE[3][2]={
{0xC8,0x08},//Jet1
{0x62,0x02},//Jet2
{0x08,0x00}//Bomb
};
liveElement Ele;

/**
  * @brief  Decode character's compact code to an array.
  * @param  decodeArr: the len=3 array to save the decode.
						compactCode: compact code defined to display on the Matrix.
  * @retval None
	*	@note		Decode the compact code by comapct rules.
	* @date		2016-01-28
  */
void decodeCharacter(unsigned char *decodeArr,unsigned char *compactCode){
	decodeArr[0] = (*compactCode << 4);
	decodeArr[1] = (*compactCode & 0xf0);
	decodeArr[2] = (*(compactCode+1) << 4);
	decodeArr[3] = (*(compactCode+1) & 0xf0);
	
}

/**
  * @brief  Generate a Layer Matrix to display (which can be displayed directly).
  * @param  layer: Pointer to the given matrix
						decodeArr: the array to be shifted. Which is generated by compact
											 code.
						coo: the position where to put the decodeArr to the layer.
  * @retval None
	*	@note		None
	* @date		2016-02-16
						2016-05-28:shiftArr(NORTH,t,8,y) -> 
						shiftArr(NORTH,t,MATRIX_X_LEN,y);
						shiftArr(SOUTH,t,8,-y) -> 
						shiftArr(SOUTH,t,MATRIX_X_LEN,-y);
  */

void generateLayer(unsigned char *layer,liveElement* ele){
	unsigned char t[MATRIX_X_LEN]={0};
	signed char x,y;
	
	x = (ele->CooX)<<4;
	y = ele->CooY;
	
	copyArray(ele->Decoded,t,4);
	
	if(x >= 0){
		shiftArr(EAST,t,MATRIX_X_LEN,x);
	}else{
		shiftArr(WEST,t,MATRIX_X_LEN,-x);
	}
	
	if(y >= 0){
		shiftArr(NORTH,t,MATRIX_X_LEN,y);
	}else{
		shiftArr(SOUTH,t,MATRIX_X_LEN,-y);
	}	
	
	copyArray(t,layer,MATRIX_X_LEN);
	
}

/**
  * @brief  Get the number of consecutive blank bits/elements from 
						arr[0] to arr[n-1],
						arr[n-1] to arr[0],
						MSB to LSB(common value of whole array),
						LSB to MSB(common value of whole array).
  * @param  decodeArr : the array to be counted.
						arrLen: the lenth of the array.
  * @retval the number of consecutive blank elements (may compated the values 
						of two oritations depends on the given parameter "ori")
	*	@note		
	* @date		2016-03-03(merged getNorthBorder/getSouthBorder/..)
						2016-05-28(modified north border algorithm under MATRIX_Y_LEN<8)
						2016-05-29(@brif rewrited. A tiny optimization of code)
*/

unsigned char getBorder(Orientation_TypeDef ori,unsigned char *decodeArr,unsigned char arrLen){
	unsigned char i,temp=0;
	unsigned char returnVal = 0;

	if(ori & N_S){
		for(i=0;i<arrLen;i++){
			temp |= decodeArr[i];
		}
		
		if(ori & NORTH){
			for(i=0;i<MATRIX_Y_LEN;i++){
				if(temp & (0x01<<(i + 8 - MATRIX_Y_LEN))){
					returnVal = i;
					break;
				}
			}
		}else{
			for(i=0;i<MATRIX_Y_LEN;i++){
				if(temp & (0x80>>i)){
					returnVal = i;
					break;
				}
			}
		}
	}
	
	if(ori & W_E){
		if(ori & WEST){
			for(i=0;i<arrLen;i++){
				if(*(decodeArr+i)){
					returnVal |= (i<<4);
					break;
				}
			}
		}else{
			for(i=0;i<arrLen;i++){
				if(*(decodeArr+arrLen-1-i)){
					returnVal |= (i<<4);
					break;
				}
			}
		}
	}
	
	return returnVal;
	
}
/**
  * @brief  Shift the whole array, specified by orientation.
  * @param  arr : the array to be shifted.
						arrLen: the lenth of the array.
						distance: the distance to shift.
  * @retval None
	*	@note		Is designed to shift the Matrix west.
	* @date		2016-03-03(merge shiftNorth/shiftSouth/SHiftWest/ShiftEast())
						
*/

void shiftArr(Orientation_TypeDef ori,unsigned char *arr,unsigned char arrLen,unsigned char shiftVals){
	signed char i,temp;
	unsigned char distance;
	
	distance = ((unsigned char)shiftVals) & 0x0f;
	if(distance==0){
		goto next;
	}
	
	if(ori & N_S){
		if(ori & NORTH){
			for(i=0;i<arrLen;i++)
				*(arr+i) >>= distance;
			
		}else{
			for(i=0;i<arrLen;i++)
				*(arr+i) <<= distance;
		}
	}
	
	next:
	distance = ((unsigned char)shiftVals) >>4;
	if(distance==0)
		return;
			
	if(ori & W_E){		
		if(ori & WEST){
			temp = (signed char)(arrLen - distance);
			if(temp>0){
				for(i=0;i<temp;i++)
					arr[i] = arr[i+distance];
			}
			clearArray(arr+temp,distance,CLR_ALL);
			
		}else{
			if(distance > arrLen)
				distance = arrLen;
			for(i=arrLen-1;i>=distance;i--)
				arr[i] = arr[i-distance];
			
			clearArray(arr,distance,CLR_ALL);
		}
	}
	
}

/**
  * @brief  Get a (pseudo) random number range from 0 to _range.
  * @param  _range: specifies the range
  * @retval None
	*	@note		TIM4 counter register acts as the random number generator.
	* @date		2016-01-14
  */
unsigned char getRandomNum(unsigned char _range){
	return TIM4_GetCounter()%_range;
}


