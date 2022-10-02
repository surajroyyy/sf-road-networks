#include "GraphData.h"

GraphData::GraphData() {}

GraphData::GraphData(string data1, string data2) {
    HSLAPixel pixel;
    ifstream ifsNode(data1);
    string nodeID, xString, yString;

    if (ifsNode.is_open()) {
        while (ifsNode >> nodeID >> xString >> yString) {
            int id = stoi(nodeID);
            double x = stod(xString);
            double y = stod(yString); //check accuracy (printf?)

            //inserting into adj list
            Node* n = new Node(id, x, y);
            adj_.push_back(n);
            numNodes_++;

            //node map implementation
            nodes_[id] = make_pair(x, y);
        }
    }

    
    ifstream ifsEdge(data2);
    string rubbish, id1, id2, distString;

    if (ifsEdge.is_open()) {
        while (ifsEdge >> rubbish >> id1 >> id2 >> distString) {
            int node1 = stoi(id1);
            int node2 = stoi(id2);
            float dist = stof(distString);

            //creating edges between nodes
            double x1 = nodes_[node1].first;
            double y1 = nodes_[node1].second;

            double x2 = nodes_[node2].first;
            double y2 = nodes_[node2].second;

            Node* curr = adj_[node1];
            while (curr->next != nullptr) curr = curr->next;
            curr->next = new Node(node2, x2, y2);

            
            curr = adj_[node2];
            while (curr->next != nullptr) curr = curr->next;
            curr->next = new Node(node1, x1, y1);

            //edge map implementation
            pair<int, int> p(node1, node2);
            pair<int, int> p2(node2, node1);
            edges_[p] = dist;
            edges_[p2] = dist;
        }
    }

}

void GraphData::insert(Node* newNode) {
    newNode = nullptr;
}

void GraphData::BFS(int id) {
    vector<bool> visited;
    for (int i = 0; i < numNodes_; i++) visited.push_back(false);

    queue<int> queue;
    visited.at(id) = true;
    queue.push(id);

    while (!queue.empty()) {
        id = queue.front();
        queue.pop();

        Node* curr = adj_[id];
        while (curr != nullptr) {
            if (visited.at(curr->id) == false) {
                visited.at(curr->id) = true;
                queue.push(curr->id);
            }
        }
    }

}

PNG GraphData::graphVisualizer() {
    const int width = 12000;
    const int height = 12000;
    PNG* vis = new PNG(width, height);

    //creating point on png for every node
    for (Node* val : adj_) {
        //creating 3 by 3 for each node for visualization
        for (int i = -1; i < 2; i++) {
            for (int j = -1; j < 2; j++) {
                if(val->id == 0) {
                    HSLAPixel & curPixel = vis->getPixel((unsigned int)(val->x + i) + 1 , (unsigned int)(vis->height() - val->y + j) - 2);
                    curPixel = GREEN;
                } else {
                    HSLAPixel & curPixel = vis->getPixel((unsigned int)(val->x + i) + 1 , (unsigned int)(vis->height() - val->y + j) - 2);
                    curPixel = ORANGE;
                }
            }
        }
    }

    map<pair<int, int>, bool> visited;
    //drawing edges between appropriate nodes with connections
    //Adapted from https://www.geeksforgeeks.org/bresenhams-line-generation-algorithm/
    //Used Bresenham’s Line Generation Algorithm
    for (Node* val : adj_) {
        Node* next_ = val->next;
        pair<int, int> p1 = make_pair(val->id, next_->id);
        pair<int, int> p2 = make_pair(next_->id, val->id);

        while (next_ != nullptr) {     
            if (!visited[p1] || !visited[p2]) { //ensuring no lines are redrawn
                double x1 = val->x + 1;
                double y1 = (vis->height() - val->y) - 2; //12000 - val bc in png 0, 0 in top left corner when should be in bottom left
                double x2 = next_->x + 1;
                double y2 = vis->height() - next_->y - 2;

                double dx = x2 - x1;
                double dy = y2 - y1;

                //Bresenham's line algo   
                //requires that first point is in bottom left and second is top right
                //so we need to swap the x and y values appropriately so algo doesnt break
                bool steep = abs(dy) > abs(dx);
                if (steep) {
                    swap(x1, y1);
                    swap(x2, y2);
                }

                if (x1 > x2) {
                    swap(x1, x2);
                    swap(y1, y2);
                }

                dx = x2 - x1;
                dy = abs(y2 - y1);

                double err = dx / 2.0;
                int y_inc = (y1 < y2) ? 1 : -1;
                int y = (int) y1;

                unsigned int x_max = (int) x2;
                for (unsigned int x = (int) x1; x <= x_max; x++) {  
                    //using alternate x or y with respect to stepp
                    if (steep) {
                        HSLAPixel & curPixel = vis->getPixel(y, x);
                        curPixel = BLUE;
                    } else {
                        HSLAPixel & curPixel = vis->getPixel(x, y);
                        curPixel = BLUE;
                    }

                    err -= dy;
                    if (err < 0) {
                        y += y_inc;
                        err += dx;
                    }
                    
                } // for loop
                visited[p1] = true;
                visited[p2] = true;
            }  // if visited
            
            next_ = next_->next;
        }
        
    }
    
    return *vis;

}


pair<vector<int>, vector<int>> GraphData::shortestPath(vector<Node*> graph, int start_id) {
    if (start_id < 0) {
        throw invalid_argument("Invalid Starting Node ID");
    }
    if ((unsigned)start_id > nodes_.size()) {
        throw invalid_argument("Invalid Starting Node ID");
    }
    vector<Node*> visited;
    vector<Node*> unvisited;
    vector<int> distances;
    vector<int> previous;
    
    for (Node* node : graph) {
        unvisited.push_back(node);
        distances.push_back(INT_MAX);
        previous.push_back(0);
    }

    distances.at(start_id) = 0;
    visited.push_back(adj_.at(start_id));
    int check_id = start_id;
    int delete_count = 0;
    while (unvisited.size() != 0) { 
        Node* check_node = adj_.at(check_id);
        pair<int, int> p;
        double min_distance = DBL_MAX;
        int lowest_neighbor = -1;
        cout << "Visited Node ID: " << check_id << endl;
        bool all_visited = true;
        while (check_node->next != nullptr) {
            check_node = check_node->next;
            p = make_pair(check_id, check_node->id);
            if (checkVisited(check_node, visited) == false) {
                all_visited = false;
                if (distances.at(check_node->id)  > edges_[p] + distances.at(check_id)) {
                    distances.at(check_node->id) = edges_[p] + distances.at(check_id);
                    previous.at(check_node->id) = check_id;  
                }
                if (edges_[p] + distances.at(check_id) < min_distance) {
                        lowest_neighbor = check_node->id;
                        min_distance = edges_[p] + distances.at(check_id); 
                }
               
            }
        }
        if (all_visited == true) {
            pair<vector<int>, vector<int>> result;
            result.first = distances;
            result.second = previous;
            return result;
        }
        check_id = lowest_neighbor; 
         
        unvisited.erase(unvisited.begin() + check_id - delete_count);
        delete_count++;
        visited.push_back(adj_.at(check_id));
    }
    pair<vector<int>, vector<int>> result;
    result.first = distances;
    result.second = previous;
    return result;
    
}

bool GraphData::checkVisited(Node* check, vector<Node*> visited) {
    for (Node* val : visited) {
        if (val->id == check->id) {
            return true;
        }
    }
    return false;
}


vector<Node*> GraphData::getAdjacencyList() {
    return adj_;
}


