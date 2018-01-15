#Logic coded by Timothy Marshall © 2017

#If we need to reset our scriptJob
def KillScript():
    try:
        if uniPivot:
             Maya.scriptJob(killAll = uniPivot, force = True)
             Maya.setAttr('square1.PointBool', 0)
        else:
           return
    except:
        return
KillScript()

#This script is called from our scriptJob
def autoPivot():
    #Query what tool the user has selected
    tool = Maya.currentCtx(q = True)
    #Maya.setToolTo(tool)
    
    #Current tool is rotate tool        
    if tool == 'RotateSuperContext':
        animPivot()

    #Current tool is move tool
    elif tool == 'moveSuperContext':
        matchScript()
    
    #Call our snapCommand() script
    snapCommand()
      
def snapCommand():    
    #list of our controls to snap to
    snapControls = ['controlX', 'controlY', 'controlZ']  
    snap = []
    
    #check if the user has a selection  
    try:
        sel = Maya.ls(sl = True)[0]

        if sel in snapControls:
            matchScript()
            loc = 'Loc%s' %sel[-1:]
            pos = Maya.xform(loc, q = True, ws = True, translation = True)
            queryRp = Maya.xform(sel, q = True, rp = True)
            for i in range(0, 3):
                snap.append((queryRp[i] + pos[i]))
            Maya.xform('controller', ws = True, translation = (snap[0], snap[1], snap[2]))
        
        else:
            return
    except:
        return

#Find the world space location of 
def matchScript():
    rp = Maya.xform('locator1', q = True, ws = True, translation = True)
    Maya.xform('square1', ws = True, translation = (rp[0], rp[1], rp[2]))
    Maya.setAttr('square1.PointBool', 1)

#Toggle the boolean of our point contraint off 
def animPivot():
    Maya.setAttr('square1.PointBool', 0)

#Resets our control to 0, 0, 0
def reset():
    Maya.setAttr('square1.translate', 0.0, 0.0, 0.0)
    Maya.setAttr('controller.rotate', 0.0, 0.0, 0.0)
    Maya.setAttr('square1.PointBool', 1)

#This calls our script and creats a job ticket for us and runs when the tool is changed
uniPivot = Maya.scriptJob(protected = True, killWithScene = True, compressUndo = True, event = ["ToolChanged","autoPivot()"])
