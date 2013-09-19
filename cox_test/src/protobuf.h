/*
 * protobuf.h
 *
 *  Created on: 2013-09-18
 *      Author: Alex
 */

#ifndef PROTOBUF_H_
#define PROTOBUF_H_

void PROTOBUF_Init();
int PROTOBUF_Append(char c);
const char *PROTOBUF_GetBuffer();
int PROTOBUF_GetLength();


#endif /* PROTOBUF_H_ */
