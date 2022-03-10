#include "globalThings.h"
//Stuff done in class with Michael Feeney

#include <sstream>
#include <iostream>
#include <physics/random_helpers.h>


// Turns off the: warning C4005: 'APIENTRY': macro redefinition
#pragma warning( disable: 4005)
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

/*static*/ void GLFW_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }


    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        // add a particle shooting mostly up
        glm::vec3 initalVelocity(nPhysics::getRandom(-2.0f, 2.0f), 10.0f, nPhysics::getRandom(-2.0f, 2.0f));
        float radius = nPhysics::getRandom(0.2f, 3.0f);
       // p->SetMass(radius * 10.0f);
        nPhysics::cParticle* p = new nPhysics::cParticle(radius * 100.0f, glm::vec3(0.0f, 1.5f, 0.0f));
        p->SetVelocity(initalVelocity);
        p->SetAcceleration(glm::vec3(0.0f, -9.8f, 0.0f));
        p->SetDamping(0.9f);
        p->SetRadius(radius);

        g_world->AddParticle(p);
        g_vec_pProjectiles.push_back(p);

        cMesh* newShot = new cMesh();
        newShot->meshName = "Isosphere_Smooth_Normals.ply";     // ISO_Sphere_flat_4div_xyz_n_rgba_uv
        newShot->positionXYZ = p->GetPosition();
        //newShot->orientationXYZ = glm::vec3(0.0f, 0.0f, 0.0f);
        newShot->scale = radius;// 1.0f;
        //newShot->bUseWholeObjectDiffuseColour = true;
        //newShot->wholeObjectDiffuseRGBA = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
        newShot->bUseObjectDebugColour = true;
        newShot->objectDebugColourRGBA = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
        newShot->bDontLight = true;
        newShot->bIsWireframe = true;
        newShot->clearTextureRatiosToZero();
      //  newShot->textureNames[1] = "2k_jupiter.bmp";
       // newShot->textureRatios[1] = 1.0f;
        ::g_vec_pMeshes.push_back(newShot);
    }

    if (key == GLFW_KEY_1 && action == GLFW_PRESS)
    {
        glm::vec3 velocity;
        float realCannonPitch = ::g_cannonPitch - (glm::pi<float>() / 2.0f);    /*cannonPitch*/
        float realCannonYaw = ::g_cannonYaw - glm::pi<float>();   /*cannonYaw*/
        velocity.z = cos(realCannonYaw) * cos(realCannonPitch);
        velocity.y = sin(realCannonPitch);
        velocity.x = sin(realCannonYaw) * cos(realCannonPitch);
        velocity = glm::normalize(velocity);
        velocity *= 20.0f;

        float radius = nPhysics::getRandom(0.2f, 3.0f);

        nPhysics::cParticle* p = new nPhysics::cParticle(radius * 10.0f, glm::vec3(0.0f, 1.0f, -15.0f));
        p->SetVelocity(velocity);
        p->SetAcceleration(glm::vec3(0.0f, -9.8f, 0.0f));
        p->SetDamping(0.5f);
        p->SetRadius(radius);
        //p->SetMass(radius * 10.0f);
        g_world->AddParticle(p);
        g_vec_pProjectiles.push_back(p);

        cMesh* newShot = new cMesh();
        newShot->meshName = "ISO_Sphere_flat_4div_xyz_n_rgba_uv.ply";
        newShot->positionXYZ = p->GetPosition();
        //newShot->orientationXYZ = glm::vec3(0.0f, 0.0f, 0.0f);
        newShot->scale = radius;// 1.0f;
      //  newShot->bUseWholeObjectDiffuseColour = false;
     //   newShot->wholeObjectDiffuseRGBA = glm::vec4(1.0f, 0.0f, .0f, 1.0f);
        newShot->bDontLight = true;
        newShot->clearTextureRatiosToZero();
        newShot->textureNames[1] = "2k_jupiter.bmp";
        newShot->textureRatios[1] = 1.0f;

        ::g_vec_pMeshes.push_back(newShot);
    }


    // From the help review session.
    // Move the camera to a location and look somewhere
    //if (key == GLFW_KEY_L && action == GLFW_PRESS)
    //{
    //    // Move the eye
    //    ::g_pFlyCamera->setEye(glm::vec3(8631.0f, -1487.0f, 13010.0f));

    //    // "look at" something.
    //    // Since we are using the fly camera, we can't just set the eye.
    //    //::g_pFlyCamera->setAt(glm::vec3(0.0f, 0.0f, 0.0f));

    //    ::g_pFlyCamera->Yaw_LeftRight(-80.0f);
    //    ::g_pFlyCamera->Pitch_up1Down(-10.0f);

    //}

    float cameraSpeed = 1.0f;
    //float objectMovementSpeed = 1.0f;
    //float lightMovementSpeed = 1.0f;

    bool bShiftDown = false;
    bool bControlDown = false;
    bool bAltDown = false;

    //    // Shift down?
    //    if ( mods == GLFW_MOD_SHIFT )       // 0x0001   0000 0001
    //    {
    //        // ONLY shift is down
    //    }
    //    // Control down?
    //    if ( mods == GLFW_MOD_CONTROL  )    // 0x0002   0000 0010
    //    // Alt down?
    //    if ( mods == GLFW_MOD_ALT   )       // 0x0004   0000 0100

        //   0000 0111 
        // & 0000 0001
        // -----------
        //   0000 0001 --> Same as the shift mask

        // Use bitwise mask to filter out just the shift
    if ((mods & GLFW_MOD_SHIFT) == GLFW_MOD_SHIFT)
    {
        // Shift is down and maybe other things, too
        bShiftDown = true;
    }
    if ((mods & GLFW_MOD_CONTROL) == GLFW_MOD_CONTROL)
    {
        // Shift is down and maybe other things, too
        bControlDown = true;
    }
    if ((mods & GLFW_MOD_ALT) == GLFW_MOD_ALT)
    {
        // Shift is down and maybe other things, too
        bAltDown = true;
    }


    //   // If JUST the shift is down, move the "selected" object
       if ( bShiftDown && (!bControlDown) && (!bAltDown) )
       {
           
    
    
       }//if ( bShiftDown && ( ! bControlDown ) && ( ! bAltDown ) )

       // If JUST the ALT is down, move the "selected" light
    if ((!bShiftDown) && (!bControlDown) && bAltDown)
    {
        

    }//if ( bShiftDown && ( ! bControlDown ) && ( ! bAltDown ) )

    return;
}
