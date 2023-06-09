#ifndef COMMANDTEMPLATE_H
#define COMMANDTEMPLATE_H

class CommandTemplate{
public:
    String Key[4];
    String Feedback[50];
    bool ReadValue;
    bool Instant;

    void function(int i = 0) = 0;
};

#endif