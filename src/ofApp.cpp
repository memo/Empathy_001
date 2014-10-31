#include "ofApp.h"

// functionality settings
#define kEndScoreThreshold  2       // end play if score less than this
#define kVideoMainFile      "video/empathy_001.mov"
#define kVideoTestFile      "video/test.mp4"
#define kDoGrabCamera       false



// display settings
#define kLargeTextSize      70

#define kScoreBoxHeight     0.1     // height of score box at the bottom of the frame (in percentage of total height)
#define kScoreColor         0xce7900

ofRectangle reportThumbsRect(0, 0.45, 1, 0.05);
ofRectangle reportGraphRect(0, 0.5, 1, 0.2);
ofRectangle reportRect(reportGraphRect.getUnion(reportThumbsRect));

#define kReportFrameHeight  0.2     // height of preview frame in report (in percentage of total height)

#define PIXH(h)             (ofGetHeight() * (h))

//--------------------------------------------------------------
// alignment: 0 -> centered, 1 -> left, -1 -> right
ofRectangle drawText(ofTrueTypeFont &f, string s, float x, float y, int hAlign = 0, int vAlign = 0) {
    ofRectangle r = f.getStringBoundingBox(s, x, y);
    switch(hAlign) {
        case 0: x = r.x - r.width/2; break;
        case 1: x = r.x; break;
        case -1: x = r.x - r.width; break;
    }
    
    switch(vAlign) {
        case 0: y = r.y - r.height/2; break;
        case 1: y = r.y; break;
        case -1: y = r.y - r.height; break;
    }
    
    
    f.drawString(s, x, y);
    return r;
}

//--------------------------------------------------------------
void ofApp::setMousePosition(float x, float y) {
    CGPoint point;
    point.x = x;
    point.y = y;
    CGWarpMouseCursorPosition(point);
    mouseX = point.x;
}


//--------------------------------------------------------------
bool ofApp::loadVideo(string filename) {
    ofLogNotice() << "loadVideo - " << filename;
    shared_ptr<ofVideoPlayer> v = shared_ptr<ofVideoPlayer>(new ofVideoPlayer());
    if(v->loadMovie(filename)) {
        v->setLoopState(OF_LOOP_NONE);
        videos[filename] = v;
        return true;
    } else {
        videos[filename] = shared_ptr<ofVideoPlayer>();
        ofLogError("   file not found");
        return false;
    }
}

//--------------------------------------------------------------
void ofApp::setVideo(string filename) {
    ofLogNotice() << "setVideo - " << filename;
    currentVideoFile = filename;
    video = videos[filename];
    if(!video) {
        ofLogWarning() << "   filename not loaded. Loading...";
        if(loadVideo(filename)) {
            setVideo(filename);
        }
    }
}


//--------------------------------------------------------------
void ofApp::setup() {
    // initialize openframeworks, opengl etc.
    ofSetFrameRate(30);
    ofBackground(255,255,255);
    ofSetVerticalSync(true);
    ofBackground(50);
    ofSetLogLevel(OF_LOG_NOTICE);
    
    // load fonts
    largeFont.loadFont("fonts/LT_51868.ttf", kLargeTextSize);
    
    bgImage.loadImage("images/bg.jpg");
    startImage.loadImage("images/start.png");
    
    // load and prepare videos
    loadVideo(kVideoMainFile);
    loadVideo(kVideoTestFile);
    
    if(kDoGrabCamera) {
        camGrabber.initGrabber(320, 240);
    }
    
    // set initial state
    setState(kStatePre);
}

//--------------------------------------------------------------
void ofApp::setState(State s) {
    state = s;
    
    switch(state) {
        case kStatePre:
            setMousePosition(ofGetWidth()/2, ofGetHeight() - PIXH(kScoreBoxHeight)/2);
            if(video) video->stop();
            break;
            
        case kStatePlaying:
            setMousePosition(ofGetWidth()/2, ofGetHeight() - PIXH(kScoreBoxHeight)/2);
            scores.clear();
            camImages.clear();
            if(video) {
                video->setPosition(0);
                video->play();
            }
            break;
            
        case kStatePost:
            if(video) video->stop();
            break;
            
        case kStateReport:
            if(video) video->stop();
            bReportDirty = true;
            reportFbo.allocate(ofGetWidth(), ofGetHeight());
            //            saveData();
            break;
            
    }
}

//--------------------------------------------------------------
void ofApp::readCurrentScore() {
    // read score: map mouse X position to 0...100
    currentScore.score = round(ofClamp(mouseX * 1.0f / ofGetWidth(), 0, 1) * 100);
    
    // read frame from video
    if(video) currentScore.frame = video->getCurrentFrame();
}


//--------------------------------------------------------------
void ofApp::saveData() {
    string name = ofSystemTextBoxDialog("Enter name for data");
    name += "-" + ofGetTimestampString();
    
    if(!name.empty()) {
        string outPath = "out/" + name;
        ofFilePath::createEnclosingDirectory(outPath);
        
        // write image
        ofImage img;
        img.grabScreen(0, PIXH(reportRect.getTop()) - 25, ofGetWidth(), PIXH(reportRect.getHeight()) + 25);    // add a bit of buffer to height
        img.saveImage(ofFilePath::join(outPath, name + ".png"));
        
        
        // write csv
        stringstream str;
        str << currentVideoFile << endl;
        for(int i=0; i<scores.size(); i++) {
            str << scores[i].frame << ", " << scores[i].score << endl;
        }
        ofBuffer b(str);
        ofBufferToFile(ofFilePath::join(outPath, name + ".csv"), b);
        
        ofSystemAlertDialog("Saved data for " + name);
    }
}

//--------------------------------------------------------------
void ofApp::loadData() {
    ofFileDialogResult f = ofSystemLoadDialog("Select CSV to load", false);
    if(f.bSuccess) {
        scores.clear();

        ofFile file(f.filePath);
        ofBuffer buffer(file);
        
        string firstLine = buffer.getFirstLine();
        setVideo(firstLine);
        
        //Read file line by line
        while (!buffer.isLastLine()) {
            string line = buffer.getNextLine();
            
            //Split line into strings
            vector<string> words = ofSplitString(line, ",");
            ScoreData scoreData;
            scoreData.frame = ofToInt(words[0]);
            scoreData.score = ofToInt(words[1]);
            scores.push_back(scoreData);
        }
        setState(kStateReport);
    }
}



//--------------------------------------------------------------
void ofApp::drawScore() {
    ofPushStyle();
    ofEnableAlphaBlending();
    ofEnableBlendMode(OF_BLENDMODE_ADD);
    ofFill();
    ofSetHexColor(kScoreColor);
    int scoreX = round(ofGetWidth() * currentScore.score * 0.01);
    ofRect(0, ofGetHeight() - PIXH(kScoreBoxHeight), scoreX, PIXH(kScoreBoxHeight));
    ofSetColor(255);
    //    drawText(smallFont, ofToString(currentScore.score), scoreX, ofGetHeight(), -1, 0);
    
    if(bShowInfo) ofDrawBitmapString(ofToString(currentScore.score), scoreX - 20, ofGetHeight() - 7);
    
    ofNoFill();
    ofSetColor(255);
    ofRect(0, ofGetHeight() - PIXH(kScoreBoxHeight), ofGetWidth(), PIXH(kScoreBoxHeight));
    ofPopStyle();
}


//--------------------------------------------------------------
void ofApp::drawReportThumbs() {
    if(video) {
        // set thumb width and height to fit window width
        float vidAspectRatio = video->getWidth() / video->getHeight();
        float th = PIXH(reportThumbsRect.getHeight());
        float tw = vidAspectRatio * th;
        int numThumbs = round(ofGetWidth() / tw);
        tw = ofGetWidth() / numThumbs;
        th = tw / vidAspectRatio;
        
        //draw thumbs
        int numFrames = video->getTotalNumFrames();
        for(int i=0; i<numThumbs; i++) {
            int frameNumber = ofMap(i, 0, numThumbs-1, 0, numFrames-1);
            video->setFrame(frameNumber);
            video->update();
            
            float x = ofMap(i, 0, numThumbs, 0, ofGetWidth());
            float y = PIXH(reportThumbsRect.getTop());
            video->draw(x, y, tw, th);
        }
    }
}

//--------------------------------------------------------------
void ofApp::drawReportScores() {
    ofPushStyle();
    
    // draw graph outline
    ofSetLineWidth(1);
    ofSetColor(0);
    ofFill();
    ofRect(0, PIXH(reportGraphRect.getTop()), ofGetWidth(), PIXH(reportGraphRect.getHeight()));
    
    // draw horizontal grid lines
    ofSetLineWidth(1);
    int numLines = 11;
    for(int i=0; i<numLines; i++) {
        ofSetColor(i % 2 ? 30 : 100);
        int ly = round(ofMap(i, 0, numLines-1, PIXH(reportGraphRect.getTop()), PIXH(reportGraphRect.getBottom())));
        ofLine(0, ly, ofGetWidth(), ly);
    }
    
    
    // draw graph of scores
    int numFrames = video ? video->getTotalNumFrames() : scores.size(); // if no video, use size of vector
    ofSetLineWidth(3);
    ofSetHexColor(kScoreColor);
    ofNoFill();
    ofBeginShape();
    for(int i=0; i<scores.size(); i++) {
        float x = ofMap(scores[i].frame, 0, numFrames, 0, ofGetWidth());
        float y = ofMap(scores[i].score, 0, 100, PIXH(reportGraphRect.getBottom()), PIXH(reportGraphRect.getTop()));
        ofVertex(x, y);
    }
    ofEndShape(false);
    
    ofPopStyle();
}

//--------------------------------------------------------------
void ofApp::drawReportFrame() {
    if(video) {
        //draw preview frame for mouse position
        int numFrames = video->getTotalNumFrames();
        int frameNumber = ofMap(mouseX, 0, ofGetWidth(), 0, numFrames);
        video->setFrame(frameNumber);
        video->update();
        
        float vidAspectRatio = video->getWidth() / video->getHeight();
        float th = PIXH(kReportFrameHeight);
        float tw = vidAspectRatio * th;
        
        float x = ofClamp(mouseX, 0, ofGetWidth() - tw);
        float y = PIXH(reportRect.getBottom());
        
        video->draw(x, y, tw, th);
        ofDrawBitmapString("Time: " + ofToString(video->getPosition() * video->getDuration(), 2) + " s", x, y + th + 15);
    }
}

//--------------------------------------------------------------
void ofApp::update(){
    switch(state) {
        case kStatePre:
            // read current score
            readCurrentScore();
            break;
            
        case kStatePlaying:
            // read current score
            readCurrentScore();
            
            // add to accumulated scores vector if we have a new score
            //            if(!scores.empty() && currentScore.score != scores.back().score)
            scores.push_back(currentScore);
            
            // update video
            if(video) {
                video->update();
                
                // if video is finished, or user wanted to end it, goto next state
                if(video->getIsMovieDone() || currentScore.score < kEndScoreThreshold) setState(kStatePost);
            }
            break;
            
        case kStatePost:
            break;
            
        case kStateReport:
            break;
    }
}

//--------------------------------------------------------------
void ofApp::draw() {
    ofSetColor(255);
    ofDisableAlphaBlending();
    bgImage.draw(0, 0, ofGetWidth(), ofGetHeight());
    
    switch(state) {
        case kStatePre:
            // draw intro texts
            ofSetColor(255);
            ofEnableAlphaBlending();
            startImage.draw(0, 0, ofGetWidth(), ofGetHeight());
            
            // draw score bar
            drawScore();
            break;
            
        case kStatePlaying:
            // draw video
            if(video) {
                ofSetColor(255);
                ofEnableAlphaBlending();
                startImage.draw(0, 0, ofGetWidth(), ofGetHeight());
                float vh = ofGetHeight() - PIXH(kScoreBoxHeight);
                float vw = vh * video->getWidth() / video->getHeight();
                video->draw(ofGetWidth()/2 - vw/2, 0, vw, vh);
            }
            
            // draw score bar
            drawScore();
            break;
            
        case kStatePost:
            // draw outro texts
            ofSetColor(255);
            drawText(largeFont, "Thank you!", ofGetWidth()/2, ofGetHeight() * 0.4);
            break;
            
        case kStateReport:
            // draw report
            ofSetColor(255);
            drawText(largeFont, "Report", ofGetWidth()/2, ofGetHeight() * 0.4);
            
            if(bReportDirty) {
                ofLogNotice() << "Report dirty, generating...";
                reportFbo.begin();
                ofClear(0, 0, 0, 0);
                drawReportThumbs();
                drawReportScores();
                reportFbo.end();
                bReportDirty = false;
            }
            
            ofEnableAlphaBlending();
            ofSetColor(255);
            reportFbo.draw(0, 0);
            
            drawReportFrame();
            break;
    }
    
    if(bShowInfo) {
        ofDrawBitmapString(ofToString(ofGetFrameRate(), 2), 10, 20);
    }
}


//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    switch(key) {
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
            setMousePosition(ofMap(key - '0', 0.5, 5.5, 0, ofGetWidth()), ofGetHeight() - PIXH(kScoreBoxHeight)/2);
            break;
            
        case 'F':
        case 'f':
            ofToggleFullscreen();
            break;
            
        case 'I':
        case 'i':
            bShowInfo ^= true;
            break;
            
        case 'S':
        case 's':
            saveData();
            break;
            
        case 'L':
        case 'l':
            loadData();
            break;
            
        case 'T':
        case 't':
            setVideo(kVideoTestFile);
            setState(kStatePlaying);
            break;
            
        case ' ' :
            switch(state) {
                case kStatePre: setVideo(kVideoMainFile); setState(kStatePlaying); break;
                case kStatePlaying: setState(kStatePost); break;
                case kStatePost: setState(kStateReport); break;
                case kStateReport: setState(kStatePre); break;
            }
    }
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {
    bReportDirty = true;
}

