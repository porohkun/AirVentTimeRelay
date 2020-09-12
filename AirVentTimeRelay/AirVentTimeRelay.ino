/*
 Name:		AirVentTimeRelay.ino
 Created:	9/11/2020 4:31:06 PM
 Author:	Poroh
*/

#include "Button.h"

#define TIME_BEFORE_AIRVENT_ON_AFTER_LIGHT_ON 120000ul
#define TIME_BEFORE_AIRVENT_OFF_AFTER_LIGHT_OFF 600000ul

#define LIGHT_BUTTON 0x03
#define AIRVT_BUTTON 0x02

#define LIGHT_RELAY 0x01
#define AIRVT_RELAY 0x00

#define STATE_IDLE       0x00
#define STATE_LIGHT_ONLY 0x01
#define STATE_BOTH       0x02
#define STATE_AIRVT_ONLY 0x03

typedef  uint8_t State;
State _state;

void setup()
{
	byte buttons[] = { LIGHT_BUTTON, AIRVT_BUTTON, 0xff,0xff };
	Button.SetButtons(buttons);

	pinMode(LIGHT_RELAY, OUTPUT);
	pinMode(AIRVT_RELAY, OUTPUT);

}

void loop()
{
	Button.Loop();
	checkTriggers();
	currentStateLoop();
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
	case STATE_IDLE:       return idleTriggers();
	case STATE_LIGHT_ONLY: return lightOnlyTriggers();
	case STATE_BOTH:        return bothTriggers();
	case STATE_AIRVT_ONLY: return airvtOnlyTriggers();
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
	if (Button.GetState(LIGHT_BUTTON))
		return STATE_LIGHT_ONLY;

	if (Button.GetState(AIRVT_BUTTON))
		return STATE_AIRVT_ONLY;

	return _state;
}

void idleLoop()
{
	digitalWrite(LIGHT_RELAY, LOW);
	digitalWrite(AIRVT_RELAY, LOW);
}

/*       ======== 'Light only' state ========       */

unsigned long _lightOnlyStartTime;
void lightOnlyStart()
{
	_lightOnlyStartTime = millis();
}

State lightOnlyTriggers()
{
	if (!Button.GetState(LIGHT_BUTTON))
		return STATE_IDLE;
	if (Button.GetState(AIRVT_BUTTON))
		return STATE_BOTH;
	if (millis() - _lightOnlyStartTime > TIME_BEFORE_AIRVENT_ON_AFTER_LIGHT_ON)
		return STATE_BOTH;

	return _state;
}

void lightOnlyLoop()
{
	digitalWrite(LIGHT_RELAY, HIGH);
	digitalWrite(AIRVT_RELAY, LOW);
}

/*       ======== 'Both' state ========       */

void bothStart()
{

}

State bothTriggers()
{
	if (!Button.GetState(LIGHT_BUTTON))
		return STATE_AIRVT_ONLY;

	return _state;
}

void bothLoop()
{
	digitalWrite(LIGHT_RELAY, HIGH);
	digitalWrite(AIRVT_RELAY, HIGH);
}

/*       ======== 'Airvent only' state ========       */

unsigned long _airvtOnlyStartTime;
void airvtOnlyStart()
{
	_lightOnlyStartTime = millis();
}

State airvtOnlyTriggers()
{
	if (Button.GetUp(AIRVT_BUTTON))
		return STATE_IDLE;
	if (millis() - _lightOnlyStartTime > TIME_BEFORE_AIRVENT_OFF_AFTER_LIGHT_OFF)
		return STATE_IDLE;

	return _state;
}

void airvtOnlyLoop()
{
	digitalWrite(LIGHT_RELAY, LOW);
	digitalWrite(AIRVT_RELAY, HIGH);
}