#pragma once

#include "ofMain.h"

class ofApp : public ofBaseApp{
public:
    enum State {
        kStatePre,
        kStatePlaying,
        kStatePost,
        kStateReport
    };
    
    struct ScoreData {
        int frame = 0;       // video frame number at which this score was recorded
        int score = 0;      // 0...100 score
    };
    

    
    bool bShowInfo = false;
    
    State state;
    
    ofTrueTypeFont largeFont;
    
    ofImage bgImage;
    ofImage startImage;
    
    shared_ptr<ofVideoPlayer> video;
    string currentVideoFile;
    map<string, shared_ptr<ofVideoPlayer> > videos;
    
    ofVideoGrabber camGrabber;
    vector<shared_ptr<ofImage> > camImages;
    
    ScoreData currentScore;     // latest score
    vector<ScoreData> scores;   // scores accumulated over the course of the video playing
    
    bool bReportDirty = true;   // whether report is dirty and needs regenerating or not
    ofFbo reportFbo;

    
    bool loadVideo(string filename);    // loads video
    void setVideo(string filename);     // sets current video to this one
    void setup();
    
    void setMousePosition(float x, float y);
    void setState(State s);
    void readCurrentScore();    // read current score from mouse position and video frame
    void saveData();
    void loadData();
    void update();              // main update

    void drawScore();           // draw score bar at bottom of screen
    void drawReportThumbs();    // draw thumbnails for report
    void drawReportScores();    // draw scores graph for report
    void drawReportFrame();     // draw current video frame for report
    void draw();                // main draw

    void keyPressed(int key);
    void windowResized(int w, int h);
};

