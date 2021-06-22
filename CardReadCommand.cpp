/*
 * CardReadCommand.cpp
 *
 *  Created on: Jun 22, 2021
 *      Author: hephaestus
 */

#include "CardReadCommand.h"

CardReadCommand::CardReadCommand() :
PacketEventAbstract(44) {

}

CardReadCommand::~CardReadCommand() {
	// TODO Auto-generated destructor stub
}

void CardReadCommand::event(float * buffer){
	buffer[0]=recentCard;
	recentCard=-1;
}
