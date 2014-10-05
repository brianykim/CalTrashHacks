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
    bool punching = false;
    bool kicking = false;
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
        if(accel.x()>=straightPunchEnd&&punching&&identifyMyo(myo)==0) {
            punching = false;
        }
        if(accel.x()<straightPunchStart&&!punching&&identifyMyo(myo)==0) {
            punching = true;
            std::cout<<"PAWWNNNNCHHHH PLAYER ONE"<<std::endl;
            myo->vibrate(myo::Myo::vibrationShort);
        }
        if(accel.x()>=-1.0&&kicking&&identifyMyo(myo)==1) {
            kicking = false;
        }
        if(accel.x()<straightPunchStart&&!kicking&&identifyMyo(myo)==1) {
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
        }
        if(roll_w >= 10&&identifyMyo(myo)==0) {
            movright = true;
            movleft = false;
        }
        if(pitch_w >= 5 && pitch_w <= 11&&identifyMyo(myo)==0) { //[~7], [8~11] [12~]
            movdown = false;
            movup = false;
        }
        if(pitch_w <= 4&&identifyMyo(myo)==0) {
            movup = true;
            movdown = false;
        }
        if(pitch_w >= 12&&identifyMyo(myo)==0) {
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
        if(identifyMyo(myo)==0) std::cout<<"0: " << acceldata<<std::endl;
        if(identifyMyo(myo)==1) std::cout<<"1: " << acceldata<<std::endl;
        
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
        if(!punching && !kicking){
            if(movleft) {
                std::cout << "welfkjwlekfjlkwejflkwejflkwjeflkweflkwjeflkjwelfkjwlfkjwelkfjweklfjlkwejfklwejf"<<std::endl;
                //here we virtually press keystrokes
                source = CGEventSourceCreate(kCGEventSourceStateCombinedSessionState);
                CGEventRef punchPress = CGEventCreateKeyboardEvent(source, (CGKeyCode)123, true);
                CGEventPost(kCGAnnotatedSessionEventTap, punchPress);
                //CGEventPost(kCGAnnotatedSessionEventTap, punchRelease);
                CFRelease(punchPress);
                CFRelease(source);
            }
            if(movright) {
                std::cout << "welfkjwlekfjlkwejflkwejflkwjeflkweflkwjeflkjwelfkjwlfkjwelkfjweklfjlkwejfklwejf"<<std::endl;
                //here we virtually press keystrokes
                source = CGEventSourceCreate(kCGEventSourceStateCombinedSessionState);
                CGEventRef punchPress = CGEventCreateKeyboardEvent(source, (CGKeyCode)124, true);
                CGEventPost(kCGAnnotatedSessionEventTap, punchPress);
                //CGEventPost(kCGAnnotatedSessionEventTap, punchRelease);
                CFRelease(punchPress);
                CFRelease(source);
            }
            if(movdown) {
                //here we virtually press keystrokes
                source = CGEventSourceCreate(kCGEventSourceStateCombinedSessionState);
                CGEventRef punchPress = CGEventCreateKeyboardEvent(source, (CGKeyCode)125, true);
                CGEventPost(kCGAnnotatedSessionEventTap, punchPress);
                //CGEventPost(kCGAnnotatedSessionEventTap, punchRelease);
                CFRelease(punchPress);
                CFRelease(source);
            }
            if(movup) {
                //here we virtually press keystrokes
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

    // onPose() is called whenever the Myo detects that the person wearing it has changed their pose, for example,
    // making a fist, or not making a fist anymore.
    void onPose(myo::Myo* myo, uint64_t timestamp, myo::Pose pose)
    {
        currentPose = pose;
        // Vibrate the Myo whenever we've detected that the user has made a fist.
        std::cout << "Myo " << identifyMyo(myo) << " switched to pose " << pose.toString() << "." << std::endl;
        
        //if (pose == myo::Pose::fist) {
        //    myo->vibrate(myo::Myo::vibrationMedium);
        //}
    }

    size_t identifyMyo(myo::Myo* myo) {
        // Walk through the list of Myo devices that we've seen pairing events for.
        for (size_t i = 0; i < knownMyos.size(); ++i) {
            // If two Myo pointers compare equal, they refer to the same Myo device.
            if (knownMyos[i] == myo) {
                return i + 1;
            }
        }
        
        return 0;
    }
    
    // onArmRecognized() is called whenever Myo has recognized a Sync Gesture after someone has put it on their
    // arm. This lets Myo know which arm it's on and which way it's facing.
    void onArmRecognized(myo::Myo* myo, uint64_t timestamp, myo::Arm arm, myo::XDirection xDirection)
    {
        onArm = true;
        whichArm = arm;
    }

    // onArmLost() is called whenever Myo has detected that it was moved from a stable position on a person's arm after
    // it recognized the arm. Typically this happens when someone takes Myo off of their arm, but it can also happen
    // when Myo is moved around on the arm.
    void onArmLost(myo::Myo* myo, uint64_t timestamp)
    {
        onArm = false;
    }

    // There are other virtual functions in DeviceListener that we could override here, like onAccelerometerData().
    // For this example, the functions overridden above are sufficient.
    // We define this function to print the current values that were updated by the on...() functions above.
    void print()
    {
        // Clear the current line
        std::cout << '\r';

        // Print out the orientation. Orientation data is always available, even if no arm is currently recognized.
        /*std::cout << '[' << std::string(roll_w, '*') << std::string(18 - roll_w, ' ') << ']'
                  << '[' << std::string(pitch_w, '*') << std::string(18 - pitch_w, ' ') << ']'
                  << '[' << std::string(yaw_w, '*') << std::string(18 - yaw_w, ' ') << ']';
         
*/
        if (onArm) {
            // Print out the currently recognized pose and which arm Myo is being worn on.

            // Pose::toString() provides the human-readable name of a pose. We can also output a Pose directly to an
            // output stream (e.g. std::cout << currentPose;). In this case we want to get the pose name's length so
            // that we can fill the rest of the field with spaces below, so we obtain it as a ssssssssssssssssssssstring using toString().
            std::string poseString = currentPose.toString();
            std:: cout << "Roll: " << roll_w << " Pitch: " << pitch_w << " Yaw: " << yaw_w <<std::endl;

            //std::cout << '[' << (whichArm == myo::armLeft ? "L" : "R") << ']'
                      //<< '[' << poseString << std::string(14 - poseString.size(), ' ') << ']';
        } else {
            // Print ousssssssssssssssssssssssssssssst a placeholder for the arm and pose when Myo doesn't currently know which arm it's on.
            //std::cout << "[?]" << '[' << std::string(14, ' ') << ']';
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
