#include "analysescore.h"
#include <cmath>


AnalyseScore::AnalyseScore()
{
//	saveTrainData = true;
	angle_bin = 2;
}

vector<Bar> AnalyseScore::Run(Mat &sheet) //main fundtion
{
    sheet_copy=sheet;

    vector<int> staff_locations= FindStave();
    CreateBars(staff_locations,  sheet);

    SegmentNotes();

    FindPrevalantEllipse(sheet);

    IdNotes();

    return bars;
}


void AnalyseScore::FindPrevalantEllipse(Mat &sheet)
{
	DataProc<float> doMath;
	GetProjection calcProj;
	vector<RotatedRect> ellipses = calcProj.DetectEllipses(sheet);
	vector<float> angles, widht, height;

	unsigned int size = ellipses.size();

	for(unsigned int i=0; i <size; i++)
	{
		angles.push_back(round(ellipses[i].angle/angle_bin));
		widht.push_back(round(ellipses[i].size.width));
		height.push_back(round(ellipses[i].size.height));
		cout<<"sz "<<ellipses[i].size<<endl;
	}

	note_angle = doMath.CalcMedian(angles);
	note_widht = doMath.CalcMedian(widht);
	note_height = doMath.CalcMedian(height);
//	waitKey();
}

 vector<int> AnalyseScore::FindStave()
{
    GetProjection findYproj;
//    imshow("sheet",sheet);
//    waitKey();

    vector<int> Y_projections = findYproj.ProjectPixels(sheet_copy,Y_axis,210);
    int avg_Y_height=findYproj.GetAvarage();

    findYproj.PlotProjections(Y_projections,"staves");

    cout<<"avg_Y_height "<<avg_Y_height<<endl;
    avg_Y_height*=3;

    vector<int> staves;

    int top_height=0, top_index;
    int staff_counter=0;

    for(unsigned int i=0;i < Y_projections.size(); i++)
    {

        if(Y_projections[i]>avg_Y_height)
        {
//            cout<<"found staff "<<endl;
            if(Y_projections[i] > top_height)
            {
                top_height=Y_projections[i];
                top_index=i;
            }
        }
        else
        {
            if(top_height>0 && top_height >( Y_projections[top_index -1] ||  Y_projections[top_index -2] )&& top_height > (Y_projections[top_index +1] ||Y_projections[top_index +2])){

                staves.push_back(top_index);
                staff_counter++;
                cout<<"new staff @ "<<   top_index<<endl;
            }
            top_height=0;
        }
    }

    cout<<"staff_counter "<<   staff_counter<<endl;
//    waitKey();
    return staves;
}



void AnalyseScore::CreateBars(vector<int> &staves, Mat &sheet_img)
{

    while(staves.size()>=4)
    {

        float bar_height= 30;//staves[4]-staves[0];
        float range=bar_height/1.5;

        Bar new_bar;
        new_bar.y_loc_start=staves[0];
        new_bar.y_loc_end=staves[4];

        for(int i=0;i<5;i++)
            new_bar.staves[i]=staves[i];

        if (staves[4] - staves[0] < (int)bar_height)
		 {
			 new_bar.bar_segment=sheet_copy.rowRange( new_bar.y_loc_start-(int)range,new_bar.y_loc_end+(int)range);
			 new_bar.avg_staff_distance = ((float)(staves[4] - staves[0]))/4;
			 new_bar.y_highest_staff = range;
			 bars.push_back(new_bar);
			 cout<<"staves "<<staves.size()<<endl;


			if(staves.size() > 4){
				staves.erase(staves.begin(),staves.begin()+4);
				imshow("bar_it ", new_bar.bar_segment);
			}
		 }
		 else
			 staves.erase(staves.begin());

        cout<<"staves "<<staves.size()<<endl;

    }
}


void AnalyseScore::SegmentNotes()
{
    int margin_note=6;
    cout<<"bars.size() "<<bars.size()<<endl;
    int note_staff=0;

    for(unsigned int i=0;i<bars.size();i++)
    {

        note_staff++;
        bars[i].GetNoteSegment( note_staff,  margin_note);

    }
    cout<<"done segnment notes "<<endl;
}



void  AnalyseScore::IdNotes()
{

	int note_counter = 200;
	NoteRecogniser Identifier;
//	Identifier.TrainSVM();
//	waitKey();

	Identifier.Train(ml_SVM);

	for(vector<Bar>::iterator bar_it=bars.begin();bar_it != bars.end() ; bar_it++)
	{
		imshow("bar_it ",bar_it->bar_segment);
		for(int i=0;i<bars.size();i++)
		{
			bool Fkey= false;
			if(i%2 != 0)
				Fkey = true;

			bars[i].GetPlayableNotes(note_widht,note_height,note_counter, Identifier,Fkey);
		}
	}
	cout<<"done ID notes"<<endl;
}

