#pragma once

#ifndef _PWMCOMMANDER_H_

void SetupPWMCommander();
void LoopPWMCommander();
void SetPWMCommand(bool direction, uint16_t pwmValue);
void SetPWMCommand(uint16_t pwmValue);

#endif //_PWMCOMMANDER_H_