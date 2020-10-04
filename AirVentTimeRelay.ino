/*
 Name:		AirVentTimeRelay.ino
 Created:	9/11/2020 4:31:06 PM
 Author:	Poroh
*/

#include "src\Button.h"

#ifndef DEBUG
//#define DEBUG
#endif

#define BTN_ACTIVE 2

#define BTN_ON_DELAY_BIT1 3
#define BTN_ON_DELAY_BIT0 5
#define BTN_OFF_DELAY_BIT1 6
#define BTN_OFF_DELAY_BIT0 7

#define BTN_LIGHT 10
#define BTN_AIRVT 11

#define REL_LIGHT 12
#define REL_AIRVT 13

#define STATE_IDLE 0x00
#define STATE_LIGHT_ONLY 0x01
#define STATE_BOTH 0x02
#define STATE_AIRVT_ONLY 0x03

typedef uint8_t State;
State _state;
unsigned long _onDelay[4] =
	{
		0ul,	   //0 min
		60000ul,   //1 min
		120000ul,  //2 min
		300000ul}; //5 min
unsigned long _offDelay[4] =
	{
		120000ul,  //2 min
		300000ul,  //5 min
		600000ul,  //10 min
		900000ul}; //15 min

void setup()
{
	byte buttons[] = {BTN_ACTIVE, BTN_ON_DELAY_BIT1, BTN_ON_DELAY_BIT0, BTN_OFF_DELAY_BIT1, BTN_OFF_DELAY_BIT0, BTN_LIGHT, BTN_AIRVT};
	Button.SetButtons(buttons);

	pinMode(REL_LIGHT, OUTPUT);
	pinMode(REL_AIRVT, OUTPUT);

#ifdef DEBUG
	Serial.begin(9600);
#endif
}

void loop()
{
	Button.Loop();
	checkTriggers();
	currentStateLoop();
}

unsigned long TimeBeforeAirventOnAfterLightOn()
{
	if (!Button.GetState(BTN_ACTIVE))
		return 0;
	byte index = (Button.GetState(BTN_ON_DELAY_BIT1) << 1) + Button.GetState(BTN_ON_DELAY_BIT0);
#ifdef DEBUG
	Serial.print("on delay: ");
	Serial.println((_onDelay[index] / 60000));
	return _onDelay[index] / 60;
#else
	return _onDelay[index];
#endif
}

unsigned long TimeBeforeAirventOffAfterLightOff()
{
	if (!Button.GetState(BTN_ACTIVE))
		return 0;
	byte index = (Button.GetState(BTN_OFF_DELAY_BIT1) << 1) + Button.GetState(BTN_OFF_DELAY_BIT0);
#ifdef DEBUG
	Serial.print("off delay: ");
	Serial.println((_offDelay[index] / 60000));
	return _offDelay[index] / 60;
#else
	return _offDelay[index];
#endif
}

void checkTriggers()
{
	State oldState = _state;
	State newState = _state;
	do
	{
		_state = newState;
		if (oldState != _state)
			currentStateStart();
		oldState = _state;
		newState = currentStateTriggers();
	} while (_state != newState);
}

void currentStateStart()
{
	switch (_state)
	{
	case STATE_IDLE:
		idleStart();
		break;
	case STATE_LIGHT_ONLY:
		lightOnlyStart();
		break;
	case STATE_BOTH:
		bothStart();
		break;
	case STATE_AIRVT_ONLY:
		airvtOnlyStart();
		break;
	}
}

State currentStateTriggers()
{
	switch (_state)
	{
	case STATE_IDLE:
		return idleTriggers();
	case STATE_LIGHT_ONLY:
		return lightOnlyTriggers();
	case STATE_BOTH:
		return bothTriggers();
	case STATE_AIRVT_ONLY:
		return airvtOnlyTriggers();
	}
}

void currentStateLoop()
{
	switch (_state)
	{
	case STATE_IDLE:
		idleLoop();
		break;
	case STATE_LIGHT_ONLY:
		lightOnlyLoop();
		break;
	case STATE_BOTH:
		bothLoop();
		break;
	case STATE_AIRVT_ONLY:
		airvtOnlyLoop();
		break;
	}
}

/*        ======== 'Idle' state ========       */

void idleStart()
{
}

State idleTriggers()
{
	if (Button.GetState(BTN_LIGHT))
		return STATE_LIGHT_ONLY;

	if (Button.GetState(BTN_AIRVT))
		return STATE_AIRVT_ONLY;

	return _state;
}

void idleLoop()
{
	digitalWrite(REL_LIGHT, LOW);
	digitalWrite(REL_AIRVT, LOW);
}

/*       ======== 'Light only' state ========       */

unsigned long _lightOnlyStartTime;
void lightOnlyStart()
{
	_lightOnlyStartTime = millis();
}

State lightOnlyTriggers()
{
	if (!Button.GetState(BTN_LIGHT))
		return STATE_IDLE;
	if (Button.GetState(BTN_AIRVT))
		return STATE_BOTH;
	if (millis() - _lightOnlyStartTime > TimeBeforeAirventOnAfterLightOn())
		return STATE_BOTH;

	return _state;
}

void lightOnlyLoop()
{
	digitalWrite(REL_LIGHT, HIGH);
	digitalWrite(REL_AIRVT, LOW);
}

/*       ======== 'Both' state ========       */

void bothStart()
{
}

State bothTriggers()
{
	if (!Button.GetState(BTN_LIGHT))
		return STATE_AIRVT_ONLY;

	return _state;
}

void bothLoop()
{
	digitalWrite(REL_LIGHT, HIGH);
	digitalWrite(REL_AIRVT, HIGH);
}

/*       ======== 'Airvent only' state ========       */

unsigned long _airvtOnlyStartTime;
void airvtOnlyStart()
{
	_lightOnlyStartTime = millis();
}

State airvtOnlyTriggers()
{
	if (Button.GetUp(BTN_AIRVT))
		return STATE_IDLE;
	if (millis() - _lightOnlyStartTime > TimeBeforeAirventOffAfterLightOff())
		return STATE_IDLE;

	return _state;
}

void airvtOnlyLoop()
{
	digitalWrite(REL_LIGHT, LOW);
	digitalWrite(REL_AIRVT, HIGH);
}