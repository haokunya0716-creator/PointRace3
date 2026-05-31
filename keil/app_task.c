#include "app_motor.h"
#include "app_vl5310x.h"
#include "clock.h"


void Task1(void){

	unsigned long task1_start_time = 0;
	mspm0_get_clock_ms(&task1_start_time);
	unsigned long task1_time = 0;
	
	while(1){
		mspm0_get_clock_ms(&task1_time);
		if(task1_time - task1_start_time < 4000 ){
		//直行
	}else if(task1_time - task1_start_time < 800){
	//左转后右转
		
	}else if(task1_time - task1_start_time < 1000){
	
		//直行
		
		break;
		}
	}
}

void Task2(void){

	
}

void Task3(void){

	
}

void Task4(void){

	
}