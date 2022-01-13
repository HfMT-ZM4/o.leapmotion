// Leapmotion object for Max/MSP
//
// Based on Jules Francois' version of aka.leapmotion.
//
// John MacCallum
// HfMT Hamburg


#include "ext.h"
#include "ext_obex.h"

#include "Leap.h"

#include <iostream>


typedef struct _oioleapmotion 
{
	t_object ob;
	Leap::Controller *leap;
	void *outlet;
	int64_t last_frame_id;
} t_oioleapmotion;

void *oioleapmotion_class;

void oioleapmotion_assist(t_oioleapmotion *x, void *b, long m, long a, char *s)
{

}

void oioleapmotion_free(t_oioleapmotion *x)
{
	delete (Leap::Controller *)(x->leap);
}

void oioleapmotion_bang(t_oioleapmotion *x)
{
	const Leap::Frame frame = x->leap->frame();
	const int64_t frame_id = frame.id();
	
	if(frame_id == x->last_frame_id){
		// output last bundle
	}
	x->last_frame_id = frame_id;
	
	const Leap::HandList hands = frame.hands();
	const size_t numHands = hands.count();
	
	t_atom frame_data[3];
	atom_setlong(frame_data, frame_id);
	atom_setlong(frame_data+1, frame.timestamp());
	atom_setlong(frame_data+2, numHands);
	outlet_anything(x->outlet, gensym("frame"), 3, frame_data);

    const Leap::GestureList &gesturelist = frame.gestures();
    const long numgestures = gesturelist.count();
	
	for(size_t i = 0; i < numHands; i++)
    {
        const int handlen = 4;
        const Leap::Hand &hand = hands[i];
        const int32_t hand_id = hand.id();
        const Leap::FingerList &fingers = hand.fingers();
        const size_t numfingers = fingers.count();
        {
            // Hand
            t_atom hand_data[handlen];
            t_atom *hdp = hand_data;
            atom_setlong(hdp++, i);
            atom_setlong(hdp++, hand_id);
            atom_setlong(hdp++, frame_id);
            atom_setlong(hdp++, numfingers);
            outlet_anything(x->outlet, gensym("hand"), handlen, hand_data);
        }
		
        for(size_t j = 0; j < numfingers; j++)
        {
            // Finger
            const int fingerlen = 17;
            const Leap::Finger &finger = fingers[j];
            const int32_t finger_id = finger.id();
            //const Leap::Ray& tip = finger.tip();
            const Leap::Vector direction = finger.direction();
            const Leap::Vector position = finger.tipPosition();
            const Leap::Vector velocity = finger.tipVelocity();
            const double width = finger.width();
            const double length = finger.length();
            const bool isTool = finger.isTool();
			
            t_atom finger_data[fingerlen];
            t_atom *fdp = finger_data;
            atom_setlong(fdp++, i); // hand index
            atom_setlong(fdp++, j); // finger index
            atom_setlong(fdp++, finger_id);
            atom_setlong(fdp++, hand_id);
            atom_setlong(fdp++, frame_id);
            atom_setfloat(fdp++, position.x);
            atom_setfloat(fdp++, position.y);
            atom_setfloat(fdp++, position.z);
            atom_setfloat(fdp++, direction.x);
            atom_setfloat(fdp++, direction.y);
            atom_setfloat(fdp++, direction.z);
            atom_setfloat(fdp++, velocity.x);
            atom_setfloat(fdp++, velocity.y);
            atom_setfloat(fdp++, velocity.z);
            atom_setfloat(fdp++, width);
            atom_setfloat(fdp++, length);
            atom_setlong(fdp++, isTool);
            outlet_anything(x->outlet, gensym("finger"), fingerlen, finger_data);
        }

        {
            const int palmlen = 15;
            const Leap::Vector position = hand.palmPosition();
            const Leap::Vector direction = hand.direction();

            t_atom palm_data[palmlen];
            t_atom *pdp = palm_data;
            atom_setlong(pdp++, i); // hand index
            atom_setlong(pdp++, hand_id);
            atom_setlong(pdp++, frame_id);
            atom_setfloat(pdp++, position.x);
            atom_setfloat(pdp++, position.y);
            atom_setfloat(pdp++, position.z);
            atom_setfloat(pdp++, direction.x);
            atom_setfloat(pdp++, direction.y);
            atom_setfloat(pdp++, direction.z);
			
            // Palm Velocity
            const Leap::Vector velocity = hand.palmVelocity();
            atom_setfloat(pdp++, velocity.x);
            atom_setfloat(pdp++, velocity.y);
            atom_setfloat(pdp++, velocity.z);
            const Leap::Vector normal = hand.palmNormal();
            atom_setfloat(pdp++, normal.x);
            atom_setfloat(pdp++, normal.y);
            atom_setfloat(pdp++, normal.z);
            outlet_anything(x->outlet, gensym("palm"), palmlen, palm_data);
        }

        {
            // Ball
            const Leap::Vector sphereCenter = hand.sphereCenter();
            const double sphereRadius = hand.sphereRadius();
			
            t_atom ball_data[6];
            atom_setlong(ball_data, hand_id);
            atom_setlong(ball_data+1, frame_id);
            atom_setfloat(ball_data+2, sphereCenter.x);
            atom_setfloat(ball_data+3, sphereCenter.y);
            atom_setfloat(ball_data+4, sphereCenter.z);
            atom_setfloat(ball_data+5, sphereRadius);
            outlet_anything(x->outlet, gensym("ball"), 6, ball_data);
        }
    }

    {
        // Gestures
        if(numgestures)
        {
            t_atom gesture_data[numgestures];
            t_atom *gdp = gesture_data;
            long numvalidgestures = numgestures;
            for(int i = 0; i < numgestures; ++i)
            {
                const Leap::Gesture g = gesturelist[i];
                switch(g.type())
                {
                case Leap::Gesture::TYPE_SWIPE:
                    // object_post((t_object *)x, "%d: swipe", i);
                    atom_setsym(gdp++, gensym("swipe"));
                    break;
                case Leap::Gesture::TYPE_CIRCLE:
                    // object_post((t_object *)x, "%d: circle", i);
                    atom_setsym(gdp++, gensym("circle"));
                    break;
                case Leap::Gesture::TYPE_SCREEN_TAP:
                    // object_post((t_object *)x, "%d: screen tap", i);
                    atom_setsym(gdp++, gensym("screen_tap"));
                    break;
                case Leap::Gesture::TYPE_KEY_TAP:
                    // object_post((t_object *)x, "%d: key tap", i);
                    atom_setsym(gdp++, gensym("key_tap"));
                    break;
                default:
                    numvalidgestures--;
                    break;
                }
            }
            outlet_anything(x->outlet, gensym("gestures"), numvalidgestures, gesture_data);
        }
    }
	
	outlet_anything(x->outlet, gensym("frame_end"), 0, nil);
}

void *oioleapmotion_new(t_symbol *s, long argc, t_atom *argv)
{
	t_oioleapmotion *x = NULL;
    
	if ((x = (t_oioleapmotion *)object_alloc((t_class *)oioleapmotion_class))){
		x->outlet = outlet_new(x, NULL);
		x->leap = new Leap::Controller;
		x->last_frame_id = 0;
        x->leap->enableGesture(Leap::Gesture::TYPE_SWIPE);
        x->leap->enableGesture(Leap::Gesture::TYPE_CIRCLE);
        x->leap->enableGesture(Leap::Gesture::TYPE_SCREEN_TAP);
        x->leap->enableGesture(Leap::Gesture::TYPE_KEY_TAP);
	}
	return x;
}

int main(void)
{	
	t_class *c;
	
	c = class_new("_.leapmotion", (method)oioleapmotion_new, (method)oioleapmotion_free, (long)sizeof(t_oioleapmotion), 0L, A_GIMME, 0);
	
	class_addmethod(c, (method)oioleapmotion_bang, "bang", 0);
	class_addmethod(c, (method)oioleapmotion_assist, "assist", A_CANT, 0);  
	
	class_register(CLASS_BOX, c);
	oioleapmotion_class = c;

	return 0;
}
