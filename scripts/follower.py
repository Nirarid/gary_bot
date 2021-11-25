#!/usr/bin/env python
import rospy
import numpy as np
from std_msgs.msg import String
from geometry_msgs.msg import Twist
from sensor_msgs.msg import LaserScan

#Commander class code
class BotFollower():
    def __init__(self):
        rospy.on_shutdown(self.terminate)
        #Instantiate publisher and subscriber  
        print("Setting up node...")
        self.pub_cmd = rospy.Publisher('cmd_follow', Twist, queue_size=10)
        print("Publishers ok")
        rospy.Subscriber("scan", LaserScan, self.follow_cb)
        print("Subscribers ok")
        print("Starting Node...")
        rate = rospy.Rate(1) #1Hz 
        print("Node initialized 1hz")
        #Initialize node variables
        self.twister = Twist()
        self.twister.linear.x = 0.0
        self.twister.angular.z = 0.0
        self.twist_str = "Bot halted"
        #Publish indicated message
        while not rospy.is_shutdown():
            self.pub_cmd.publish(self.twister)
            rospy.loginfo(self.twist_str)
            rate.sleep()
    
    #Process received command
    def follow_cb(self,msg):
        closest = min(msg.ranges)
        ind = msg.ranges.index(closest)
        if np.isfinite(closest):
            #Angle calculus and twisting
            theta = msg.angle_min + ind*msg.angle_increment
            self.twister.angular.z = 0.45*theta
            #Distance calculus and twisting
            if theta < 0.18 and theta > -0.18:
                if closest < 0.8:
                    self.twister.linear.x = 0.0
                    self.twist_str = "Within safety region"
                elif closest > 3.0:
                    self.twister.linear.x = 1.0
                    self.twist_str = "Reached top speed"
                else:
                    self.twister.linear.x = 0.333*closest
                    self.twist_str = "Approaching closest object"
            else:
                self.twister.linear.x = 0.0
                self.twist_str = "Turning bot"
        else:
            self.twister.linear.x = 0.0
            self.twister.angular.z = 0.0
            self.twist_str = "Bot halted"
     
    #Process node shutdown (called when node is killed)
    def terminate(self):
        self.twister.linear.x = 0.0
        self.twister.angular.z = 0.0
        rospy.loginfo("Terminating node...")
        
#Main code
if __name__ == "__main__":
    rospy.init_node("obj_follow",anonymous=True)
    try:
    	BotFollower()
    except:
        rospy.logfatal("obj_follow node died")
        
