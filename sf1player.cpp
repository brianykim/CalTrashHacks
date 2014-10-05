// Copyright (C) 2013-2014 Thalmic Labs Inc.
// Distributed under the Myo SDK license agreement. See LICENSE.txt for details.
#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <string>
#include <sstream>
#include <vector>
#include <ApplicationServices/ApplicationServices.h>
#include <unistd.h>

// The only file that needs to be included to use the Myo C++ SDK is myo.hpp.
#include <myo/myo.hpp>

using std::cout;
using std::endl;

const int runFreq =5;
// Classes that inherit from myo::DeviceListener can be used to receive events from Myo devices. DeviceListener
// provides several virtual functions for handling different kinds of events. If you do not override an event, the
// default behavior is to do nothing.
class DataCollector : public myo::DeviceListener {
public:
    CGEventSourceRef source;

    std::string acceldata;
    const double straightPunchStart = -2.5;
    const double straightPunchEnd = -0.5;
    const int hadoukenStart = 4; // forward = 8 (+/-5; - is RIGHT)
    const int hadoukenEnd = 6;
    bool hadouken = false;
    bool punching = false;
    bool kicking = false;
    bool lastdirleft = false; //(P1 default faces right first)
    bool movleft, movright, movdown, movup = false;
    std::vector<myo::Myo*> knownMyos;
    DataCollector()
    : onArm(false), roll_w(0), pitch_w(0), yaw_w(0), currentPose()
    {
        acceldata="";
    }
    //std::string acceldata = "";

    void onPair(myo::Myo* myo, uint64_t timestamp, myo::FirmwareVersion firmwareVersion)
    {
        knownMyos.push_back(myo);
        std::cout << "Paired with " << identifyMyo(myo) << "." << std::endl;
    }
    
    void onConnect(myo::Myo* myo, uint64_t timestamp, myo::FirmwareVersion firmwareVersion)
    {
        std::cout << "Myo " << identifyMyo(myo) << " has connected." << std::endl;
    }
    
    void onDisconnect(myo::Myo* myo, uint64_t timestamp)
    {
        std::cout << "Myo " << identifyMyo(myo) << " has disconnected." << std::endl;
    }
    
    // onUnpair() is called whenever the Myo is disconnected from Myo Connect by the user.
    void onUnpair(myo::Myo* myo, uint64_t timestamp)
    {
        // We've lost a Myo.
        // Let's clean up some leftover state.
        roll_w = 0;
        pitch_w = 0;
        yaw_w = 0;
        onArm = false;
    }
    
    void onAccelerometerData(myo::Myo* myo, uint64_t timestamp, const myo::Vector3<float>& accel)
    {
        /*if(yaw_w>=hadoukenEnd&&hadouken&&identifyMyo(myo)==0){
            hadouken = false;
        }
        if(currentPose.toString() == "waveOut"&&yaw_w<hadoukenStart&&!hadouken&&identifyMyo(myo)==0){
            hadouken = true;
        }*/
        if(accel.x()>=straightPunchEnd&&punching&&identifyMyo(myo)==0) {
            punching = false;
        }
        if(accel.x()<straightPunchStart&&!punching&&identifyMyo(myo)==0) {
            punching = true;
            std::cout<<"PAWWNNNNCHHHH PLAYER ONE"<<std::endl;
            myo->vibrate(myo::Myo::vibrationShort);
        }
        
        if((accel.x()>=-1.0/*||pitch_w>=14*/)&&kicking&&identifyMyo(myo)==1) {
            kicking = false;
        }
        if((accel.x()<straightPunchStart/*||pitch_w<=13*/)&&!kicking&&identifyMyo(myo)==1) {
            kicking = true;
            std::cout<<"KIIIIIICK PLAYER ONE"<<std::endl;
        }
        if(roll_w >= 6 && roll_w <= 10&&identifyMyo(myo)==0) { //[4,6], [7,9] [10,12]
            movleft = false;
            movright = false;
        }
        if(roll_w <= 6&&identifyMyo(myo)==0) {
            movleft = true;
            movright = false;
            lastdirleft = true;
        }
        if(roll_w >= 10&&identifyMyo(myo)==0) {
            movright = true;
            movleft = false;
            lastdirleft = false;
        }
        if(pitch_w >= 8 && pitch_w <= 12&&identifyMyo(myo)==0) { //[~7], [8~11] [12~]
            movdown = false;
            movup = false;
        }
        if(pitch_w <= 7&&identifyMyo(myo)==0) {
            movup = true;
            movdown = false;
        }
        if(pitch_w >= 13&&identifyMyo(myo)==0) {
            movdown = true;
            movup = false;
        }
        std::ostringstream strs;
        strs << accel.x();
        std::string str = strs.str();
        acceldata =  str;
        acceldata += ", ";
        strs << accel.y();
        str = strs.str();
        acceldata += str;
        acceldata += ", ";
        strs << accel.z();
        str = strs.str();
        acceldata += str;
        //if(identifyMyo(myo)==0) std::cout<<"0: " << acceldata<<std::endl;
        //if(identifyMyo(myo)==1) std::cout<<"1: " << acceldata<<std::endl;
        /*if(hadouken) {
            CGEventRef LRArrowPress;
            CGEventRef LRArrowRelease;
            CGEventRef LRArrowPress2;
            CGEventRef LRArrowRelease2;
            source = CGEventSourceCreate(kCGEventSourceStateCombinedSessionState);
            CGEventRef downArrowPress = CGEventCreateKeyboardEvent(source, (CGKeyCode)125, true);
            CGEventPost(kCGAnnotatedSessionEventTap, downArrowPress);
            CGEventRef downArrowRelease = CGEventCreateKeyboardEvent(source, (CGKeyCode)125, false);
            CGEventPost(kCGAnnotatedSessionEventTap, downArrowRelease);
            if(lastdirleft){
                LRArrowPress = CGEventCreateKeyboardEvent(source, (CGKeyCode)123, true);
                CGEventPost(kCGAnnotatedSessionEventTap, LRArrowPress);
                LRArrowRelease = CGEventCreateKeyboardEvent(source, (CGKeyCode)123, false);
                CGEventPost(kCGAnnotatedSessionEventTap, LRArrowRelease);
                LRArrowPress2 = CGEventCreateKeyboardEvent(source, (CGKeyCode)123, true);
                CGEventPost(kCGAnnotatedSessionEventTap, LRArrowPress2);
                LRArrowRelease2 = CGEventCreateKeyboardEvent(source, (CGKeyCode)123, false);
                CGEventPost(kCGAnnotatedSessionEventTap, LRArrowRelease2);
            }else{
                LRArrowPress = CGEventCreateKeyboardEvent(source, (CGKeyCode)124, true);
                CGEventPost(kCGAnnotatedSessionEventTap, LRArrowPress);
                LRArrowRelease = CGEventCreateKeyboardEvent(source, (CGKeyCode)124, false);
                CGEventPost(kCGAnnotatedSessionEventTap, LRArrowRelease);
                LRArrowPress2 = CGEventCreateKeyboardEvent(source, (CGKeyCode)124, true);
                CGEventPost(kCGAnnotatedSessionEventTap, LRArrowPress2);
                LRArrowRelease2 = CGEventCreateKeyboardEvent(source, (CGKeyCode)124, false);
                CGEventPost(kCGAnnotatedSessionEventTap, LRArrowRelease2);
            }
            CGEventRef aPress = CGEventCreateKeyboardEvent(source, (CGKeyCode)13, true);
            CGEventPost(kCGAnnotatedSessionEventTap, aPress);
            CGEventRef aRelease = CGEventCreateKeyboardEvent(source, (CGKeyCode)13, false);
            CGEventPost(kCGAnnotatedSessionEventTap, aRelease);
            
            CFRelease(downArrowPress);
            CFRelease(LRArrowPress);
            CFRelease(downArrowRelease);
            CFRelease(LRArrowRelease);
            CFRelease(LRArrowPress2);
            CFRelease(LRArrowRelease2);
            CFRelease(aPress);
            CFRelease(aRelease);
            cout << "Hadoukened" << endl;
            hadouken = false;
        }
        if(!hadouken){
            //zzzzzz neeeds moar hadouken
        }*/
        if(punching){
            source = CGEventSourceCreate(kCGEventSourceStateCombinedSessionState);
            CGEventRef punchPress = CGEventCreateKeyboardEvent(source, (CGKeyCode)13, true);
            CGEventPost(kCGAnnotatedSessionEventTap, punchPress);
            CFRelease(punchPress);
            
            CFRelease(source);
        }
        if(!punching){
            source = CGEventSourceCreate(kCGEventSourceStateCombinedSessionState);
            CGEventRef punchRelease = CGEventCreateKeyboardEvent(source, (CGKeyCode)13, false);
            CGEventPost(kCGAnnotatedSessionEventTap, punchRelease);
            CFRelease(punchRelease);
            CFRelease(source);
        }
        
        if(kicking){
            source = CGEventSourceCreate(kCGEventSourceStateCombinedSessionState);
            CGEventRef kickPress = CGEventCreateKeyboardEvent(source, (CGKeyCode)2, true);
            CGEventPost(kCGAnnotatedSessionEventTap, kickPress);
            CFRelease(kickPress);
            CFRelease(source);
        }
        if(!kicking){
            source = CGEventSourceCreate(kCGEventSourceStateCombinedSessionState);
            CGEventRef kickRelease = CGEventCreateKeyboardEvent(source, (CGKeyCode)2, false);
            CGEventPost(kCGAnnotatedSessionEventTap, kickRelease);
            CFRelease(kickRelease);
            CFRelease(source);
        }
        if(!punching && !kicking && !hadouken){
            if(movleft) {
                source = CGEventSourceCreate(kCGEventSourceStateCombinedSessionState);
                CGEventRef punchPress = CGEventCreateKeyboardEvent(source, (CGKeyCode)123, true);
                CGEventPost(kCGAnnotatedSessionEventTap, punchPress);
                CFRelease(punchPress);
                CFRelease(source);
            }
            if(movright) {
                source = CGEventSourceCreate(kCGEventSourceStateCombinedSessionState);
                CGEventRef punchPress = CGEventCreateKeyboardEvent(source, (CGKeyCode)124, true);
                CGEventPost(kCGAnnotatedSessionEventTap, punchPress);
                CFRelease(punchPress);
                CFRelease(source);
            }
            if(movdown) {
                source = CGEventSourceCreate(kCGEventSourceStateCombinedSessionState);
                CGEventRef punchPress = CGEventCreateKeyboardEvent(source, (CGKeyCode)125, true);
                CGEventPost(kCGAnnotatedSessionEventTap, punchPress);
                CFRelease(punchPress);
                CFRelease(source);
            }
            if(movup) {
                source = CGEventSourceCreate(kCGEventSourceStateCombinedSessionState);
                CGEventRef punchPress = CGEventCreateKeyboardEvent(source, (CGKeyCode)126, true);
                CGEventRef punchRelease = CGEventCreateKeyboardEvent(source, (CGKeyCode)126, false);
                CGEventPost(kCGAnnotatedSessionEventTap, punchPress);
                CGEventPost(kCGAnnotatedSessionEventTap, punchRelease);
                CFRelease(punchRelease);
                CFRelease(punchPress);
                CFRelease(source);
            }
            if(!movright){
                source = CGEventSourceCreate(kCGEventSourceStateCombinedSessionState);
                CGEventRef punchRelease = CGEventCreateKeyboardEvent(source, (CGKeyCode)124, false);
                CGEventPost(kCGAnnotatedSessionEventTap, punchRelease);
                CFRelease(punchRelease);
                CFRelease(source);
            }
            if(!movleft){
                source = CGEventSourceCreate(kCGEventSourceStateCombinedSessionState);
                CGEventRef punchRelease = CGEventCreateKeyboardEvent(source, (CGKeyCode)123, false);
                CGEventPost(kCGAnnotatedSessionEventTap, punchRelease);
                CFRelease(punchRelease);
                CFRelease(source);
            }
            if(!movdown){
                source = CGEventSourceCreate(kCGEventSourceStateCombinedSessionState);
                CGEventRef punchRelease = CGEventCreateKeyboardEvent(source, (CGKeyCode)125, false);
                CGEventPost(kCGAnnotatedSessionEventTap, punchRelease);
                CFRelease(punchRelease);
                CFRelease(source);
            }
        }
    }
    // onOrientationData() is called whenever the Myo device provides its current orientation, which is represented as a unit quaternion.
    void onOrientationData(myo::Myo* myo, uint64_t timestamp, const myo::Quaternion<float>& quat)
    {
        using std::atan2;
        using std::asin;
        using std::sqrt;
        // Calculate Euler angles (roll, pitch, and yaw) from the unit quaternion.
        float roll = atan2(2.0f * (quat.w() * quat.x() + quat.y() * quat.z()),
                           1.0f - 2.0f * (quat.x() * quat.x() + quat.y() * quat.y()));
        float pitch = asin(2.0f * (quat.w() * quat.y() - quat.z() * quat.x()));
        float yaw = atan2(2.0f * (quat.w() * quat.z() + quat.x() * quat.y()),
                        1.0f - 2.0f * (quat.y() * quat.y() + quat.z() * quat.z()));
        // Convert the floating point angles in radians to a scale from 0 to 20.
        roll_w = static_cast<int>((roll + (float)M_PI)/(M_PI * 2.0f) * 18);
        pitch_w = static_cast<int>((pitch + (float)M_PI/2.0f)/M_PI * 18);
        yaw_w = static_cast<int>((yaw + (float)M_PI)/(M_PI * 2.0f) * 18);
    }
    void onPose(myo::Myo* myo, uint64_t timestamp, myo::Pose pose)
    {
        currentPose = pose;
        //std::cout << "Myo " << identifyMyo(myo) << " switched to pose " << pose.toString() << "." << std::endl;
    }

    size_t identifyMyo(myo::Myo* myo) {
        for (size_t i = 0; i < knownMyos.size(); ++i) {
            if (knownMyos[i] == myo) {
                return i + 1;
            }
        }
        
        return 0;
    }
    void onArmRecognized(myo::Myo* myo, uint64_t timestamp, myo::Arm arm, myo::XDirection xDirection)
    {
        onArm = true;
        whichArm = arm;
    }
    void onArmLost(myo::Myo* myo, uint64_t timestamp)
    {
        onArm = false;
    }
    void print()
    {
        // Clear the current line
        //std::cout << '\r';
        if (onArm) {
            std::string poseString = currentPose.toString();
            //std:: cout << "Roll: " << roll_w << " Pitch: " << pitch_w << " Yaw: " << yaw_w <<std::endl;
        }
        if(punching || kicking) {
        //std::cout << acceldata << std::endl;
        }
        std::cout << std::flush;
    }
    // These values are set by onArmRecognized() and onArmLost() above.
    bool onArm;
    myo::Arm whichArm;
    // These values are set bys sonOrientationData() ansdss onPose() above.
    int roll_w, pitch_w, yaw_w;
    myo::Pose currentPose;
};


int main(int argc, char** argv)
{
    try {
        // First, we create a Hub with our application identifier. Be sure not to use the com.example namespace when
        // publishing your application. The Hub provides access to one or more Myos.
        myo::Hub hub("com.example.hello-myo");
        std::cout << "Attempting to find a Myo..." << std::endl;
        // Next, we attempt to find a Myo to use. If a Myo is already paired in Myo Connect, this will return that Myo
        // immediately.
        myo::Myo* myo = hub.waitForMyo(10000);
        if (!myo) {
            throw std::runtime_error("Unable to find a Myo!");
        }
        std::cout << "Connected to a Myo armband!" << std::endl << std::endl;
        DataCollector collector;

        // Hub::addListener() takes the address of any object whose class inherits from DeviceListener, and will cause
        // Hub::run() to send events to all registered device listeners.
        hub.addListener(&collector);
        while (1) { // main loop
            // In each iteration of our main loop, we run the Myo event loop for a set number of milliseconds.
            // Display update frequency
            hub.run(runFreq);
            // After processing events, call print() to print.
            collector.print();
            
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        std::cerr << "Press enter to continue.";
        std::cin.ignore();
        return 1;
    }
}