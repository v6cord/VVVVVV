#ifndef ENT_H
#define ENT_H

class entclass
{
public:
    entclass();

    void clear();

    bool outside();

public:
    //Fundamentals
    bool active, invis = false;
    int type, size, tile, rule = 0;
    int state, statedelay = 0;
    int behave, animate = 0;
    float para = 0.0;
    int life, colour = 0;

    //Position and velocity
    int oldxp, oldyp = 0;
    float ax, ay, vx, vy = 0.0;
    int cx, cy, w, h = 0;
    float newxp, newyp = 0.0;
    bool isplatform = false;
    int x1,y1,x2,y2 = false;
    //Collision Rules
    int onentity = false;
    bool harmful = 0;
    int onwall, onxwall, onywall = 0;

    //Platforming specific
    bool jumping = false;
    bool gravity = false;
    int onground, onroof = 0;
    int jumpframe = 0;
    //Animation
    int framedelay, drawframe, walkingframe, dir, actionframe = 0;
    int yp, xp = 0;
};

#endif /* ENT_H */
