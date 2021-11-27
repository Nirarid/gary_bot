#!/usr/bin/env python
import rospy
import numpy as np
from std_msgs.msg import Int32
from geometry_msgs.msg import Twist

class Multiplexer():
    def __init__(self):
        rospy.on_shutdown(self.terminate)
        #Instantiate publisher and subscriber  
        print("Setting up node...")
        self.pub_cmd = rospy.Publisher('arduino/cmd_vel', Twist, queue_size=10)
        print("Publishers ok")
        rospy.Subscriber("cmd_follow", Twist, self.follow_msg_cb)
        rospy.Subscriber("cmd_teleop", Twist, self.teleop_msg_cb)
        rospy.Subscriber("cmd_avoid", Twist, self.avoid_msg_cb)
        rospy.Subscriber("cmd_mode", Int32, self.mode_msg_cb)
        print("Subscribers ok")
        print("Starting Node...")
        rate = rospy.Rate(1) #1Hz 
        print("Node initialized 1hz")
        #Initialize node variables
        self.mode = Int32()
        self.Ftwister = Twist()
        self.Ftwister.linear.x = 0.0
        self.Ftwister.angular.z = 0.0
        self.Ttwister = Twist()
        self.Ttwister.linear.x = 0.0
        self.Ttwister.angular.z = 0.0
        self.Atwister = Twist()
        self.Atwister.linear.x = 0.0
        self.Atwister.angular.z = 0.0
        #Publish indicated message
        while not rospy.is_shutdown():
            if(self.mode == 0):
                self.pub_cmd.publish(self.Ttwister)
            elif(self.mode == 1):
                self.pub_cmd.publish(self.Ftwister)
            else:
                self.pub_cmd.publish(self.Atwister)
            rate.sleep()
    
    #Process received command from follower
    def follow_msg_cb(self,msg):
        self.Ftwister.angular.z = msg.angular.z
        self.Ftwister.linear.x = msg.linear.x
        
    #Process received command from follower
    def teleop_msg_cb(self,msg):
        self.Ttwister.angular.z = msg.angular.z
        self.Ttwister.linear.x = msg.linear.x
    
    #Process received command from follower
    def avoid_msg_cb(self,msg):
        self.Atwister.angular.z = msg.angular.z
        self.Atwister.linear.x = msg.linear.x
    
    #Process received command from follower
    def mode_msg_cb(self,msg):
        self.mode = msg.data
    
    #Process node shutdown (called when node is killed)
    def terminate(self):
        self.twister.linear.x = 0.0
        self.twister.angular.z = 0.0
        rospy.loginfo("Terminating node...")


#Main code
if __name__ == "__main__":
    rospy.init_node("msg_multiplex",anonymous=True)
    try:
        Multiplexer()
    except:
        rospy.logfatal("msg_multiplexer node died")
