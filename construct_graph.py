#!/usr/bin/python
# -*- coding:utf-8 -*-

import re
import math
import copy
import sys
import errno

pointID = {}

def GiveIDtoPoint(point):
    if pointID.has_key(point):
        point.setID(pointID[point])
    else:
        point.setID(len(pointID.keys()) + 1)
        pointID[point] = point.id

class Point:
    def __init__(self, x, y):
        self.x = float(x)
        self.y = float(y)
        self.id = -1  # initial id
        self.isIntersection = False  # whether intersection or not, initial value is flase

    def setID(self, id):
        self.id = id

    def markAsIntersec(self):
        self.isIntersection = True

    def __str__(self):
        return "  %s:\t(%.2f,%.2f)" % (self.id, self.x, self.y)

    def __hash__(self):
        return ("x:%s,y:%s" % (self.x, self.y)).__hash__()

    def __eq__(self, other):
        return self.x == other.x and self.y == other.y

def CalculateDistances(pA, pB):
    return math.sqrt((pA.x - pB.x) ** 2 + (pA.y - pB.y) ** 2)

def GetIntersection(segA, segB):
    l1 = Line(segA)
    l2 = Line(segB)
    D = l1.a * l2.b - l2.a * l1.b
    if D == 0:
        return None # if D = 0, line1 == line2
    else:
        inter_x = (l1.b * l2.c - l2.b * l1.c) / D
        inter_y = (l1.c * l2.a - l2.c * l1.a) / D
        inter = Point(inter_x, inter_y)
        dAP1 = CalculateDistances(segA.pA, inter)
        dPB1 = CalculateDistances(inter, segA.pB)
        dAB1 = CalculateDistances(segA.pA, segA.pB)
        dAP2 = CalculateDistances(segB.pA, inter)
        dPB2 = CalculateDistances(inter, segB.pB)
        dAB2 = CalculateDistances(segB.pA, segB.pB)
        if (dAB1 >= dAP1 and dAB1 >= dPB1) and (dAB2 >= dAP2 and dAB2 >= dPB2):
            return inter
    return None

class Segment:
    def __init__(self, pA, pB): # point A, point B
        self.pA = pA
        self.pB = pB

    def __eq__(self, other):
        return self.pA == other.pA and self.pB == other.pB

    def __hash__(self):
        return self.__str__().__hash__()

    def __str__(self):
        return "<%s,%s>" % (self.pA.id - 1, self.pB.id - 1)

def isStreetNameValid(StreetName): # only allow letters and space characters
    '''pattern = r'[a-zA-Z\s]'
    matchObj = re.match(pattern, StreetName)
    if matchObj:
        return StreetName
    else:
        print("Error: street name's format is wrong")'''

class Street:
    def __init__(self, name, points):
        self.name = name
        self.Line = []  # segments list
        self.points = []   # points list
        self.InitSegmentsPoints(points)

    def segments(self):
        return self.Line

    def InitSegmentsPoints(self, points):
        if len(points) >= 2: # at least two points made up a segment
            for i in range(len(points) - 1):
                self.Line.append(Segment(points[i], points[i + 1]))
            self.points = points
        else:
            print("Error: you need to input at least two points")

    def AddPointIntoSegment(self, segment, pP):
        if (segment.pA == pP) or (segment.pA == pP):
            if segment.pA == pP:
                segment.pA.isIntersection = True
            if segment.pB == pP:
                segment.pB.isIntersection = True
            return None # if it is end point, cannot be added
        SegA = Segment(segment.pA, pP)
        SegB = Segment(pP, segment.pB) # after adding a point, turn the old segment into two new segments
        NewSegs = []
        for i in range(len(self.Line)):
            if self.Line[i] != segment:
                NewSegs.append(self.Line[i])
            else:
                NewSegs.append(SegA)
                NewSegs.append(SegB)
        self.Line = NewSegs

        NewPoints = []
        for i in range(len(self.points)):
            NewPoints.append(self.points[i])
            if self.points[i] == segment.pA:
                if NewPoints.count(pP) == 0:  # not exist
                    NewPoints.append(pP)
        self.points = NewPoints

def DrawPoints(pointListString): # draw point from RE pattern
    points = []
    pattern = re.compile(r'\((\-?\d+),(\-?\d+)\)')
    for p in pattern.findall(str(pointListString).strip()): # remove head and tail
        points.append(Point(int(p[0]), int(p[1])))
    return points

def OrderSegment(pA, pB):
    if pA.id > pB.id:
        pC = pA
        pA = pB
        pB = pC
    return Segment(pA, pB)

class Line:
    def __init__(self, seg):
        self.p1 = seg.pA
        self.p2 = seg.pB
        self.a = self.p1.y - self.p2.y
        self.b = self.p2.x - self.p1.x
        self.c = self.p1.x * self.p2.y - self.p1.y * self.p2.x

class Graph:
    def __init__(self):
        self.streets = {}
        self.FinalPoint = {}
        '''
        include:
        a.each intersection
        b.the end-point of a line segment of a street that intersects with another street
        '''
        self.edges = {}
        '''
        include
        a.at least one of them is an intersection
        b.both lie on the same street
        c.one is reachable from the other without traversing another vertex
        '''
    def AddFinalPoint(self, point):
        if not self.FinalPoint.has_key(point):  # vertify point not in final results
            GiveIDtoPoint(point)
            self.FinalPoint[point] = point

    def AddEdge(self, edge):
        if not self.edges.has_key(edge): # vertify the edge does exist and did not add before
            if edge.pA != edge.pB:
                self.edges[edge] = edge

    def AddStreet(self, command):
        pattern = r'a *\"(.+?)\"(( ?\(\-?\d+,\-?\d+\))+)\s*$'
        matchObj = re.match(pattern, command)
        if matchObj: # meet the requirement
            streetName = matchObj.group(1).upper()
            isStreetNameValid(streetName)
            if self.streets.has_key(streetName):
                return
            pointListString = matchObj.group(2)
            points = DrawPoints(pointListString)
            self.streets[streetName] = Street(streetName, points)
        else:
            print("Error: your input format is wrong")

    def RemoveStreet(self, command):
        pattern = r'r \"(.+?)\"'
        matchObj = re.match(pattern, command)
        if matchObj:
            streetName = matchObj.group(1).upper()
            isStreetNameValid(streetName)
            if self.streets.has_key(streetName):  # if exsit, delete
                del self.streets[streetName]
            else:
                print("Error: this street is not exist, cannot be removed")
        else:
            print("Error: your input format is wrong")

    def ChangeStreet(self, command):
        pattern = r'c *\"(.+?)\"(( ?\(\-?\d+,\-?\d+\))+)\s*$'
        matchObj = re.match(pattern, command)
        if matchObj:
            streetName = matchObj.group(1).upper()
            isStreetNameValid(streetName)
            if self.streets.has_key(streetName): # only exsit street can be chaxnged
                pointListString = matchObj.group(2)
                points = DrawPoints(pointListString)
                self.streets[streetName] = Street(streetName, points)
            else:
                print("Error: this street is not exist, cannot be changed")
        else:
            print("Error: your input format is wrong")

    def generate(self):
        initG = copy.deepcopy(G)
        SNames = initG.streets.keys()
        for i in range(len(SNames) - 1): # start from first street's first segment
            for j in range(i + 1, len(SNames)): # compare with next street's first segment
                s1 = initG.streets[SNames[i]]
                s2 = initG.streets[SNames[j]]
                for segA in s1.segments():
                    for segB in s2.segments():
                        inter = GetIntersection(segA, segB)
                        if inter:
                            inter.markAsIntersec()
                            s1.AddPointIntoSegment(segA, inter)
                            s2.AddPointIntoSegment(segB, inter)

        for i in range(len(SNames)):
            S = initG.streets[SNames[i]]
            for j in range(len(S.points)):
                if S.points[j].isIntersection:
                    initG.AddFinalPoint(S.points[j])
                    if j - 1 >= 0:
                        initG.AddFinalPoint(S.points[j - 1])
                        edge = OrderSegment(initG.FinalPoint[S.points[j - 1]], initG.FinalPoint[S.points[j]])
                        initG.AddEdge(edge)
                    if j + 1 < len(S.points):
                        initG.AddFinalPoint(S.points[j + 1])
                        edge = OrderSegment(initG.FinalPoint[S.points[j]], initG.FinalPoint[S.points[j + 1]])
                        initG.AddEdge(edge)

        # refer: https://www.runoob.com/python/att-string-format.html
        # refer: https://stackoverflow.com/questions/10019456/usage-of-sys-stdout-flush-method
        sys.stdout.write('V {}\n'.format(len(initG.FinalPoint)))
        sys.stdout.flush()

        sys.stdout.write('E {')
        sys.stdout.flush()

        count = 0
        for e in initG.edges:
            count += 1
            if count < len(initG.edges):
                sys.stdout.write('{},'.format(e))
                sys.stdout.flush()
            else:  # last one don't need to print ","
                sys.stdout.write(str(e))
                sys.stdout.flush()
        sys.stdout.write('}\n')
        sys.stdout.flush()

        pointID.clear()
        '''
        if len(initG.FinalPoint) == 0:
            sys.stdout.flush()
        else:
            sys.stdout.write('V {}\n'.format(len(initG.FinalPoint)))
            sys.stdout.flush()

            sys.stdout.write('E {')
            sys.stdout.flush()

            count = 0
            for e in initG.edges:
                count += 1
                if count < len(initG.edges):
                    sys.stdout.write('{},'.format(e))
                    sys.stdout.flush()
                else:  # last one don't need to print ","
                    sys.stdout.write(str(e))
                    sys.stdout.flush()
            sys.stdout.write('}\n')
            sys.stdout.flush()

        pointID.clear()
        '''
G = Graph()

while True:
    try:
        command = raw_input()
        if command[0] == "a":
            G.AddStreet(command)
        elif command[0] == "c":
            G.ChangeStreet(command)
        elif command[0] == "r":
            G.RemoveStreet(command)
        elif command[0] == "g":
            G.generate()
        else:
            print("Error: command input wrong, only allowed a, c, r, g")
    except EOFError:
        sys.exit()
    except IOError as e:
        if e.errno == errno.EPIPE:
            sys.exit()

