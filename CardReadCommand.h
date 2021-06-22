/*
 * CardReadCommand.h
 *
 *  Created on: Jun 22, 2021
 *      Author: hephaestus
 */

#ifndef CARDREADCOMMAND_H_
#define CARDREADCOMMAND_H_

#include <PacketEvent.h>

class CardReadCommand: public PacketEventAbstract {

public:
	CardReadCommand();
	virtual ~CardReadCommand();
	void event(float * buffer);
	float recentCard=-1;
};

#endif /* CARDREADCOMMAND_H_ */
