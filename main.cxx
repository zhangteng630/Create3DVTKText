#include <iostream>
#include <map>
#include <vector>

#include <vtkSmartPointer.h>
#include <vtkActor.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkVectorText.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkPolyData.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkCell.h>
#include <vtkTriangle.h>
#include <vtkTriangleFilter.h>
#include <vtkProperty.h>
#include <vtk-6.3/vtkRenderWindowInteractor.h>

vtkSmartPointer< vtkPolyData > Create3DVTKText(vtkStdString str, double depth = 0.5)
{
	//construct t0 (the front surface)
	vtkSmartPointer< vtkVectorText > vectorText = vtkSmartPointer< vtkVectorText >::New();
	vectorText->SetText(str.c_str());
	vectorText->Update();
	vtkSmartPointer< vtkTriangleFilter > triangleFilter = vtkSmartPointer< vtkTriangleFilter >::New();
	triangleFilter->SetInputData(vectorText->GetOutput());
	triangleFilter->Update();
	vtkSmartPointer< vtkPolyData > t0 = triangleFilter->GetOutput();
	//translate t0 (to get the back surface)
	vtkSmartPointer< vtkTransform > transform = vtkSmartPointer< vtkTransform >::New();
	transform->Translate(0.0, 0.0, depth);
	vtkSmartPointer< vtkTransformPolyDataFilter > transformFilter = vtkSmartPointer< vtkTransformPolyDataFilter >::New();
	transformFilter->SetInputData(t0);
	transformFilter->SetTransform(transform);
	transformFilter->Update();
	vtkSmartPointer< vtkPolyData > t1 = transformFilter->GetOutput();
	//extract outer lines
	std::map< std::pair< vtkIdType, vtkIdType >, std::vector< vtkIdType > > lineCellsMap;
	for (vtkIdType cid = 0; cid < t0->GetNumberOfCells(); ++cid)
	{
		vtkCell * cell = t0->GetCell(cid);
		for (vtkIdType p0 = 0; p0 < 2; ++p0)
		{
			for (vtkIdType p1 = 1; p1 < 3; ++p1)
			{
				vtkIdType pid0 = cell->GetPointId(p0);
				vtkIdType pid1 = cell->GetPointId(p1);
				if (pid0 > pid1)
					std::swap(pid0, pid1);
				std::pair< vtkIdType, vtkIdType > line = std::pair< vtkIdType, vtkIdType >(pid0, pid1);
				std::map< std::pair< vtkIdType, vtkIdType >, std::vector< vtkIdType > >::iterator iter = lineCellsMap.find(line);
				if (iter == lineCellsMap.end())
				{
					std::vector< vtkIdType > vec;
					vec.push_back(cid);
					lineCellsMap.insert(std::pair< std::pair< vtkIdType, vtkIdType >, std::vector< vtkIdType > >(line, vec));
				}
				else
				{
					(*iter).second.push_back(cid);
				}
			}
		}
	}
	std::vector< std::pair< vtkIdType, vtkIdType > > outerLines;
	for (std::map< std::pair< vtkIdType, vtkIdType >, std::vector< vtkIdType > >::iterator iter = lineCellsMap.begin();
		iter != lineCellsMap.end(); ++iter)
	{
		if ((*iter).second.size() == 1)
		{
			outerLines.push_back((*iter).first);
		}
	}
	//build 3D text
	vtkSmartPointer< vtkPoints > points = vtkSmartPointer< vtkPoints >::New();
	vtkSmartPointer< vtkCellArray > cells = vtkSmartPointer< vtkCellArray >::New();
	points->DeepCopy(t0->GetPoints());
	cells->DeepCopy(t0->GetPolys());
	for (vtkIdType pid = 0; pid < t0->GetNumberOfPoints(); ++pid)
	{
		points->InsertNextPoint(t1->GetPoint(pid));
	}
	for (vtkIdType cid = 0; cid < t0->GetNumberOfCells(); ++cid)
	{
		vtkCell * cell = t0->GetCell(cid);
		vtkSmartPointer< vtkTriangle > triangle = vtkSmartPointer< vtkTriangle >::New();
		for (vtkIdType ptId = 0; ptId < cell->GetNumberOfPoints(); ++ptId)
		{
			triangle->GetPointIds()->SetId(ptId, cell->GetPointId(ptId) + t0->GetNumberOfPoints());
		}
		cells->InsertNextCell(triangle);
	}
	for (size_t i = 0; i < outerLines.size(); ++i)
	{
		vtkIdType pid00 = outerLines[i].first;
		vtkIdType pid01 = outerLines[i].second;
		vtkIdType pid10 = pid00 + t0->GetNumberOfPoints();
		vtkIdType pid11 = pid01 + t0->GetNumberOfPoints();
		vtkSmartPointer< vtkTriangle > triangle0 = vtkSmartPointer< vtkTriangle >::New();
		triangle0->GetPointIds()->SetId(0, pid00);
		triangle0->GetPointIds()->SetId(1, pid01);
		triangle0->GetPointIds()->SetId(2, pid11);
		cells->InsertNextCell(triangle0);
		vtkSmartPointer< vtkTriangle > triangle1 = vtkSmartPointer< vtkTriangle >::New();
		triangle1->GetPointIds()->SetId(0, pid00);
		triangle1->GetPointIds()->SetId(1, pid10);
		triangle1->GetPointIds()->SetId(2, pid11);
		cells->InsertNextCell(triangle1);
	}
	vtkSmartPointer< vtkPolyData > text = vtkSmartPointer< vtkPolyData >::New();
	text->SetPoints(points);
	text->SetPolys(cells);
	
	return text;
}

int main(int argc, char * argv[])
{
	vtkStdString str("Welcome to GitHub!");
	if (argc > 1)
	{
		str = vtkStdString(argv[1]);
	}
	
	vtkSmartPointer< vtkPolyDataMapper > mapper = vtkSmartPointer< vtkPolyDataMapper >::New();
	mapper->SetInputData(Create3DVTKText(str));
	vtkSmartPointer< vtkActor > actor = vtkSmartPointer< vtkActor >::New();
	actor->SetMapper(mapper);
	vtkSmartPointer< vtkRenderer > ren = vtkSmartPointer< vtkRenderer >::New();
	ren->AddActor(actor);
	vtkSmartPointer< vtkRenderWindow > win = vtkSmartPointer< vtkRenderWindow >::New();
	win->AddRenderer(ren);
	vtkSmartPointer< vtkRenderWindowInteractor > interactor = vtkSmartPointer< vtkRenderWindowInteractor >::New();
	interactor->SetRenderWindow(win);
	
	win->Render();
	interactor->Start();
	
	return 0;
}
