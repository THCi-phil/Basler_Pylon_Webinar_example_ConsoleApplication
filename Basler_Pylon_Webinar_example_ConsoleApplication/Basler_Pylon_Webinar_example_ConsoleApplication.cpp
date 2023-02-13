// Basler_Pylon_Webinar_example_ConsoleApplication.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <filesystem>
#include <chrono>

#include <pylon/PylonIncludes.h>
#include <pylon/PylonGUI.h>

//using namespace Pylon;    //Basler Pylon proprietary API for their cameras (but may work for other GenIcam standard compliant camera from other vendors)
//using namespace GenApi;   //the GeniCam API, needed to access the camera settings
using namespace std;

std::filesystem::path parentDirectoryPath ;  //needs C++17 enabled
std::filesystem::path resultsDirectoryPath;
std::filesystem::path rawImageDataDirectoryPath;


bool setToHardwareTriggerFrameStart( Pylon::CInstantCamera& camera );
bool initialiseResultsDirectoryStructure();


int main()
{
    Pylon::PylonInitialize();
    
    try {
        Pylon::CInstantCamera cameraROJER_1_Basler( Pylon::CTlFactory::GetInstance().CreateFirstDevice() );
        //defaults to free run, so we have to set it to hardware triggered
        if ( !setToHardwareTriggerFrameStart( cameraROJER_1_Basler ) ) { return -1; }
        if ( !initialiseResultsDirectoryStructure()                  ) { return -1; };
        
        cameraROJER_1_Basler.StartGrabbing(100);
        Pylon::CGrabResultPtr camGrabPtr_image_ROJER_1_Basler;

        Pylon::String_t fileName_commonRoot = "firstTestROJERcamPylon_f";
        Pylon::String_t fileName_extension  = ".tif";
        
        std::filesystem::current_path( rawImageDataDirectoryPath );
        while ( cameraROJER_1_Basler.IsGrabbing() ) {
            cameraROJER_1_Basler.RetrieveResult( 3000, camGrabPtr_image_ROJER_1_Basler );
            int64_t  grabbedFrameNumber    = camGrabPtr_image_ROJER_1_Basler->GetID();
            uint64_t grabbedFrameTimeTicks = camGrabPtr_image_ROJER_1_Basler->GetTimeStamp();
            cout << "frame no. " << grabbedFrameNumber << ", time stamp = " << grabbedFrameTimeTicks << " ticks" << endl;
            Pylon::DisplayImage( 1, camGrabPtr_image_ROJER_1_Basler);
            //Pylon::CImagePersistence::Save second parameter is Pylon:String_t&, see definition not documentation (which erroneously has it as std::string&)
            //Pylon::String_t accepts a const char array as an input. Tediously, convert number to an std::string and then pass in string.c_str() to the String_t constructor. 
            Pylon::String_t fileName = fileName_commonRoot + Pylon::String_t( std::to_string(grabbedFrameNumber).c_str() ) + fileName_extension;
            Pylon::CImagePersistence::Save(  Pylon::ImageFileFormat_Tiff, fileName, camGrabPtr_image_ROJER_1_Basler );
        }
    } //end try
    catch ( Pylon::GenericException  exception ) {
        cout << "Basler camera, Pylon API exception: " << exception.what() << endl;
        return -1;
    } //end catch
	cin.get(); //waits for a key press, so the Pylon image window isn't cleared until we can play with it
    Pylon::PylonTerminate();
    return 0;
}
//end int  main()
//-----------------------------------------------------------------



bool initialiseResultsDirectoryStructure() {
	auto timeNow = std::chrono::system_clock::now();

    parentDirectoryPath       = std::filesystem::current_path()       ;
    resultsDirectoryPath      = parentDirectoryPath  / "results"      ;
    rawImageDataDirectoryPath = resultsDirectoryPath / "rawImageData" ;

    if ( !std::filesystem::exists( resultsDirectoryPath      )) std::filesystem::create_directory( resultsDirectoryPath      );
    if ( !std::filesystem::exists( rawImageDataDirectoryPath )) std::filesystem::create_directory( rawImageDataDirectoryPath );

    cout << "parent directory                              = " << parentDirectoryPath       << endl ;
    cout << "results saved in directory                    = " << resultsDirectoryPath      << endl ;
    cout << "raw image data from camera saved in directory = " << rawImageDataDirectoryPath << endl ;
    
    return true;
} //end bool initialiseResultsDirectoryStructure()
//-----------------------------------------------------------------




bool setToHardwareTriggerFrameStart(Pylon::CInstantCamera& camera ) {
    try {
        if( !camera.IsOpen() ) { camera.Open(); }  //settings aren't writeable until the camera is opened (but also must not yet have started grabbing, for most settings)
        GenApi::INodeMap& nodeMap = camera.GetNodeMap();
        GenApi::CEnumerationPtr triggerSelector( nodeMap.GetNode( "TriggerSelector" ) );
        GenApi::CEnumerationPtr triggerMode(     nodeMap.GetNode( "TriggerMode"     ) );
        if ( GenApi::IsImplemented(triggerSelector) && GenApi::IsImplemented(triggerMode)
          && GenApi::IsAvailable  (triggerSelector) && GenApi::IsAvailable  (triggerMode)
          && GenApi::IsWritable   (triggerSelector) && GenApi::IsWritable   (triggerMode)
           ) {
            Pylon::CEnumParameter( nodeMap, "TriggerSelector" ).SetValue( "FrameStart" );
            Pylon::CEnumParameter( nodeMap, "TriggerMode"     ).SetValue( "On"         );
            return true;
        }
        else {
            cout << "fatal error: camera hardware trigger cannot be set." << endl;
            return false;
        }
    } //end try
    catch (Pylon::GenericException  exception) {
        cout << "fatal error: camera hardware trigger cannot be set: " << exception.what() << endl;
        return false;
    } //end catch
} //end int  setToHardwareTriggerFrameStart(Pylon::CInstantCamer)
//-----------------------------------------------------------------



// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
