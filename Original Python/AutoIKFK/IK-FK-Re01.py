#Auto IK<->FK Switch Script. Coded by Timothy Marshall ©2017

#Lists of control object in scene by TYPE {Real or Fake}
#This will need to be replaced with dynamic list objects later
FK = ['|FK_Shoulder_Grp_01|FK_Shouder_CTRL_01', '|FK_Shoulder_Grp_01|FK_Shouder_CTRL_01|FK_Elbow_Grp_01|FK_Elbow_CTRL_01', '|FK_Shoulder_Grp_01|FK_Shouder_CTRL_01|FK_Elbow_Grp_01|FK_Elbow_CTRL_01|FK_Wrist_Grp_01|FK_Wrist_CTRL_01']
IK = ['|IK_Wrist_Grp_01|IK_Wrist_CTRL_01', '|IK_PoleVector_Grp_01|IK_PoleVector_CTRL_01']
FakeFK = ['|Bind_Shoulder_JNT_01|FK_Shoulder_Grp_01|FK_Shouder_CTRL_01', '|Bind_Shoulder_JNT_01|Bind_Elbow_JNT_01|FK_Elbow_Grp_01|FK_Elbow_CTRL_01', '|Bind_Shoulder_JNT_01|Bind_Elbow_JNT_01|Bind_Wrist_JNT_01|FK_Wrist_Grp_01|FK_Wrist_CTRL_01']
FakeIK = ['|FK_Shoulder_Grp_01|FK_Shouder_CTRL_01|FK_Elbow_Grp_01|FK_Elbow_CTRL_01|FK_Wrist_Grp_01|FK_Wrist_CTRL_01|IK_Wrist_CTRL_01', '|FK_Shoulder_Grp_01|FK_Shouder_CTRL_01|IK_PoleVector_Grp_01|IK_PoleVector_CTRL_01']

#List of ALL control objeects in our scene.
AllControls = FK + IK + FakeFK + FakeIK

#Turn off our script and sets everything to default
def KillScript():
    try:
        Maya.scriptJob(kill = script, force = True)
        SystemOff()
    except:
        print('\nScript not running'),
KillScript()

#We're moving from IK --> FK, Matching FK to Fake counterparts
#So when we switch the [FK] will be where the IK was last.
def IKtoFK():

    # 1 - Loop through our list and match objects rotations {FK has no translations}
    for i, e in enumerate(FakeFK):
        Roto = Maya.xform(e, q = True, ws = True, rotation = True)
        Maya.xform(FK[i], ws = True, rotation = (Roto[0], Roto[1], Roto[2]))
        
        # 2 - Toggle visibility of IK objects from real to Fakes
        Maya.setAttr('%s.visibility' %e, 0)
        Maya.setAttr('%s.visibility' %FK[i], 1)
    
    #3 - Toggle our blend
    Maya.setAttr('IK_FK_Offset_CTRL_01.IK_FK_Blend', 0)
        
        
    for i, e in enumerate(IK):
        Maya.setAttr('%s.visibility' %FakeIK[i], 1)
        Maya.setAttr('%s.visibility' %e, 0)
    
    #Matching up the location and orientation of the IK Pol Vectors
    TransPV = Maya.xform(IK[1], q = True, ws = True, translation = True)
    Maya.xform(FakeIK[1], ws = True, translation = (TransPV[0], TransPV[1], TransPV[2]))
    RotoPV = Maya.xform(IK[1], q = True, ws = True, rotation = True)
    Maya.xform(FakeIK[1], ws = True, rotation = (RotoPV[0], RotoPV[1], RotoPV[2]))

#We're moving from FK --> IK Matching IK to Fake counterparts
#So when we switch the IK will be where the FK was last.
def FKtoIK():

    # 1 - Loop through our list and match objects locations and rotations    
    for i, e in enumerate(FakeIK):
        Trans = Maya.xform(e, q = True, ws = True, translation = True)
        Maya.xform(IK[i], ws = True, translation = (Trans[0], Trans[1], Trans[2]))
        Roto = Maya.xform(e, q = True, ws = True, rotation = True)
        Maya.xform(IK[i], ws = True, rotation = (Roto[0], Roto[1], Roto[2]))
        
        #2 - Toggle visibility of FK objects from real to Fakes
        Maya.setAttr('%s.visibility' %e, 0)
        Maya.setAttr('%s.visibility' %IK[i], 1)
        Maya.setAttr('%s.visibility' %FakeFK[i], 1)
        Maya.setAttr('%s.visibility' %FK[i], 0)
        
    #3 - Toggle our blend
    Maya.setAttr('IK_FK_Offset_CTRL_01.IK_FK_Blend', 1)

    for i, e in enumerate(FakeFK):
        Maya.setAttr('%s.visibility' %e, 1)
        #Maya.setAttr('%s.visibility' %[i], 0)
            
    #Matching up the location and orientation of the IK Pol Vectors
    TransPV = Maya.xform(IK[1], q = True, ws = True, translation = True)
    Maya.xform(FakeIK[1], ws = True, translation = (TransPV[0], TransPV[1], TransPV[2]))
    RotoPV = Maya.xform(IK[1], q = True, ws = True, rotation = True)
    Maya.xform(FakeIK[1], ws = True, rotation = (RotoPV[0], RotoPV[1], RotoPV[2]))
        
#This function acts like an Emergency Brake turning Everything Auto -> OFF
def SystemOff():
    for i, e in enumerate(FK):
        Maya.setAttr('%s.visibility' %e, 1)
        Maya.setAttr('%s.visibility' %FakeFK[i], 0)
    
    for i, e in enumerate(FakeIK): 
        Maya.setAttr('%s.visibility' %FakeIK[i], 0)
        Maya.setAttr('%s.visibility' %IK[i], 1)
  
#This is our script that gets called everytime
def AutoIKFK():
    try:
        #Get the name of user selection and format it for use later
        sel = Maya.ls(sl = True, long = True)[0].encode("utf-8")
        print('\n%s' %sel),
    
        #Check if there is a selection and it is in our defined list above
        if (sel in AllControls):
            print('\nMatch'),
            
            if sel in FK or sel in FakeFK:
                for i, e in enumerate(FakeFK):
                    if sel == FakeFK[i]:
                        Maya.select(FK[i], r = True)
                IKtoFK() 
                       
            if sel in IK or sel in FakeIK:
                for i, e in enumerate(FakeIK):       
                    if sel == FakeIK[i]:
                        Maya.select(IK[i], r = True)
                FKtoIK()
        
        else:
            SystemOff() 

    except:        
        pass
        #print('\nError: Nothing selected..'),
        
#This calls our script and creats a job ticket for us
script = Maya.scriptJob(protected = True, killWithScene = True, compressUndo = True, event= ["SelectionChanged","AutoIKFK()"])
