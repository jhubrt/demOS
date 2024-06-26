#ifndef SNDTRACK_H
#define SNDTRACK_H

#define SAMPLE_L	89008UL			
#define SAMPLE_S	44504UL

s16 playTrack (void);

enum PlayerStep_
{
    STEP_SOUNDTRACK_STARTED  = 1,
    STEP_START_POLYZOOM      ,
    STEP_POLYZOOM_STARTED    ,
    STEP_POLYZOOM_STOP       ,
    STEP_START_VISUALIZER    ,
    STEP_STOP_VISUALIZER     ,
    STEP_VISUALIZER_STOPPED  ,
    STEP_START_INTERLUDE1    ,
    STEP_INTERLUDE1_STOPPED  ,
    STEP_START_VISUALIZER2   ,
    STEP_STOP_VISUALIZER2    ,
    STEP_VISUALIZER2_STOPPED ,
    STEP_INTERLUDE2_STOPPED  ,
    STEP_START_VISUALIZER3   ,
    STEP_STOP_VISUALIZER3    ,
    STEP_VISUALIZER3_STOPPED ,
    STEP_START_SLIDES        ,
    STEP_SLIDES_STARTED      ,
    STEP_SLIDES_NEAREND      ,
    STEP_SLIDES_STOPPED      ,
    STEP_START_VISUALIZER4   ,
    STEP_STOP_VISUALIZER4    ,
    STEP_VISUALIZER4_STOPPED ,
    STEP_START_FUGIT        
};
typedef enum PlayerStep_ PlayerStep;

#endif
