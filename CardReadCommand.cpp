/*
 * CardReadCommand.cpp
 *
 *  Created on: Jun 22, 2021
 *      Author: hephaestus
 */

#include "CardReadCommand.h"
#include <Arduino.h>
CardReadCommand::CardReadCommand() :
PacketEventAbstract(44) {

}

CardReadCommand::~CardReadCommand() {
	// TODO Auto-generated destructor stub
}

void CardReadCommand::event(float * buffer){
	if(recentCard>0){
		Serial.println("processing card "+String(recentCard));
	}
	buffer[0]=recentCard;
	buffer[1]=-2;
	buffer[2]=-3;
	recentCard=-1;
}
