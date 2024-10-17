#include <iostream>
#include <queue>
#include <algorithm>  // std::shuffle
#include <random>     // std::random_device, std::mt19937
#include <list>

#include <vector>

const int MaxKeyNum = 5;  // B+树的阶数

struct Node {
    Node* parent;

    Node* children[MaxKeyNum + 1];  // 子节点指针
    int keys[MaxKeyNum];            // 关键字数组
    int nkey;                       // 当前关键字数量
    bool isLeaf;                    // 是否是叶子节点
    Node* next;                     // 叶子节点指向下一个叶子节点的指针

    Node(bool leaf = false) {
        parent = nullptr;//空
        nkey = 0;
        isLeaf = leaf;
        next = nullptr;
        for (int i = 0; i <= MaxKeyNum; i++) {
            children[i] = nullptr;
        }
    }
};





struct BPlusTree {
    Node* root;

    BPlusTree() {
        root = new Node(true);  // 初始时，根节点是叶子节点
    }

    // 找到适当的叶子节点来插入数据
    Node* findLeafNode(int key, std::vector<Node*>& path) {
        Node* current = root;
        // 現在のノードが葉ノードでない間、繰り返し探索する
        path.push_back(current);
        while (!current->isLeaf) {
            int i = 0;
            // 現在のノードのキー配列の中で、key よりも大きなキーを持つ位置を探す
            while (i < current->nkey && key >= current->keys[i]) {
                i++;// 次のキーを確認するためにインデックスを増加させる
            }
            // 子ノードの中で、キーが見つかる範囲の子へ移動する
            current = current->children[i];
            path.push_back(current);

        }
        // 葉ノードに到達したので、そのノードを返す
        return current;
    }

    /*
    // 搜索函数，先打印找到的叶子节点，然后再打印找到的键
    void search(int key) {
        std::vector<Node*> path;
        // 1. 找到包含目标键的叶子节点，并记录遍历路径
        Node* leaf = findLeafNode(key, path);

        // 2. 打印路径中的所有节点
        for (Node* node : path) {
            printNode(node);  // 打印每个节点的键
        }

        // 3. 在叶子节点中查找目标键
        bool found = false;
        for (int i = 0; i < leaf->nkey; i++) {
            std::cout << "Compare " << key << " with " << leaf->keys[i] << std::endl;
            if (leaf->keys[i] == key) {
                found = true;
                break;
            }
        }

        // 4. 根据查找结果输出消息
        if (found) {
            std::cout << "Key " << key << " found." << std::endl;
        }
        else {
            std::cout << "Key " << key << " not found." << std::endl;
        }
    }
    */

    // 検索関数、成功したら true を返す
    bool search(int key) {
        std::vector<Node*> path;
        Node* leaf = findLeafNode(key,path) ;

        for (int i = 0; i < leaf->nkey; i++) {
            if (leaf->keys[i] == key) {
                std::cout << "Key " << key << " found." << std::endl;
                return true;
            }
        }

        std::cout << "Key " << key << " not found." << std::endl;
        return false;
    }





    // 插入数据
    void insert(int key) {
        std::vector<Node*> path;
        // 1. 找到适当的叶子节点来插入数据
        Node* leaf = findLeafNode(key, path);

        // 2. 找到插入位置
        int i = leaf->nkey - 1;   while (i >= 0 && key < leaf->keys[i]) {
            leaf->keys[i + 1] = leaf->keys[i];
            i--;
        }
        // 3. 插入新键值
        leaf->keys[i + 1] = key;
        leaf->nkey++;//挿入したので、葉ノードのキー数を1つ増やす。

        // 4. 如果叶子节点已满，进行分裂
        if (leaf->nkey == MaxKeyNum) {
            splitLeafNode(leaf);
        }
    }

    // 分裂叶子节点
    void splitLeafNode(Node* leaf) {
        // 1. 计算中间位置。对于阶数为 MaxKeyNum 的 B+ 树，中间索引为 MaxKeyNum / 2
        int mid = MaxKeyNum / 2;

        // 2. 创建一个新的叶子节点 newLeaf，并设置为叶节点类型
        Node* newLeaf = new Node(true);

        // 3. 将右半部分的键值从原叶子节点移动到新的叶子节点
        //mid 之后的键将移入 newLeaf，更新键的数量 (nkey)
        for (int i = mid; i < MaxKeyNum; i++) {
            newLeaf->keys[i - mid] = leaf->keys[i]; // leaf の右半分のキーを newLeaf に移動
            newLeaf->nkey++; // 增加 newLeaf 的键数量
            leaf->nkey--; // 减少原 leaf 的键数量
        }

        // 4. 更新叶子节点链表的链接
        newLeaf->next = leaf->next;// 新叶子节点指向原叶子节点的下一个节点
        leaf->next = newLeaf; // 原叶子节点指向新的叶子节点

        // 5. 将新叶子节点的第一个键值提升到父节点进行插入
        if (leaf->parent == nullptr) {
            // 如果当前叶子节点没有父节点，说明是根节点，需要创建一个新的根节点
            root = new Node(false); // 创建一个新的根节点，非叶节点
            root->keys[0] = newLeaf->keys[0];  // 将 newLeaf 的第一个键提升到根节点
            root->children[0] = leaf;// 将原叶子节点作为第一个子节点
            root->children[1] = newLeaf; // 将新叶子节点作为第二个子节点
            root->nkey++; // 更新根节点的键数量
            leaf->parent = root;// 设置原叶子节点的父节点为新根节点
            newLeaf->parent = root; // 设置新叶子节点的父节点为新根节点
        }
        else {
            // 如果有父节点，将新叶子节点的第一个键插入到父节点
            insertInternal(leaf->parent, newLeaf->keys[0], newLeaf);
        }
    }

    // 将中间值插入到内部节点
    void insertInternal(Node* parent, int key, Node* newChild) {
        // 从父节点的最后一个键开始，寻找新键插入的位置
        int i = parent->nkey - 1;
        // 当新键小于当前父节点的键时，向右移动键和子节点指针，腾出空间插入新键
        while (i >= 0 && key < parent->keys[i]) {
            parent->keys[i + 1] = parent->keys[i];// 将当前键向右移动一位
            parent->children[i + 2] = parent->children[i + 1];// 将对应的子节点指针向右移动
            i--;
        }
        // 找到插入位置后，将新键插入父节点
        parent->keys[i + 1] = key;
        // 插入新的子节点指针
        parent->children[i + 2] = newChild;
        parent->nkey++; // 父节点的键数量加一
        newChild->parent = parent; // 设置新子节点的父节点指针

        // 如果父节点已满（键数量达到最大），需要继续分裂父节点
        if (parent->nkey == MaxKeyNum) {
            splitInternalNode(parent);
        }
    }

    // 分裂内部节点
    void splitInternalNode(Node* node) {
        // 计算中间位置，分裂时将中间键提升到父节点
        int mid = MaxKeyNum / 2;
        // 创建一个新的内部节点
        Node* newInternal = new Node(false);

        // 将右半部分的键和子节点从当前节点移动到新节点
        for (int i = mid + 1; i < MaxKeyNum; i++) {
            newInternal->keys[i - (mid + 1)] = node->keys[i];// 将右半部分的键移到新节点
            newInternal->children[i - (mid + 1)] = node->children[i];// 移动相应的子节点指针
            newInternal->children[i - (mid + 1)]->parent = newInternal;// 更新新子节点的父节点指针
            newInternal->nkey++; // 增加新节点的键数量
            node->nkey--;  // 减少原节点的键数量
        }

        // 将最后一个子节点指针也移到新节点
        newInternal->children[newInternal->nkey] = node->children[MaxKeyNum];
        newInternal->children[newInternal->nkey]->parent = newInternal;
        node->nkey--;  // 中间值将被提升到父节点，原节点的键数量减少

        int midKey = node->keys[mid];  // 中间值

        // 如果当前节点是根节点（没有父节点），需要创建一个新的根节点
        if (node->parent == nullptr) {
            root = new Node(false);// 创建一个新的根节点
            root->keys[0] = midKey;// 创建一个新的根节点
            root->children[0] = node;// 原节点作为新根节点的第一个子节点
            root->children[1] = newInternal;// 新节点作为新根节点的第二个子节点
            root->nkey++; // 增加新根节点的键数量
            node->parent = root;// 更新原节点的父节点指针
            newInternal->parent = root;// 更新新节点的父节点指针
        }
        else {
            // 如果当前节点有父节点，将中间键和新节点插入到父节点
            insertInternal(node->parent, midKey, newInternal);
        }
    }


    // 打印节点的所有键
    void printNode(Node* node, bool isLeaf = false) {
        if (isLeaf) {
            std::cout << "node: [";
        }
        else {
            std::cout << "node: [";
        }
        for (int i = 0; i < node->nkey; i++) {
            std::cout << node->keys[i];
            if (i != node->nkey - 1) {
                std::cout << " ";  // 键之间的空格
            }
        }
        std::cout << "]" << std::endl;
    }


    // 按层打印树结构，并且叶子节点按顺序输出
    void printTreeByLevel() {
        if (root == nullptr) return;

        std::queue<Node*> q;
        q.push(root);

        while (!q.empty()) {
            int size = q.size();  // 当前层的节点数量

            for (int i = 0; i < size; i++) {
                Node* node = q.front();
                q.pop();

                // 打印当前节点的键
                std::cout << "[";
                for (int j = 0; j < node->nkey; j++) {
                    std::cout << node->keys[j];
                    if (j != node->nkey - 1)
                        std::cout << " ";
                }
                std::cout << "] ";

                // 如果当前节点不是叶子节点，将其子节点加入队列
                if (!node->isLeaf) {
                    for (int j = 0; j <= node->nkey; j++) {
                        q.push(node->children[j]);
                    }
                }
            }
            std::cout << std::endl;  // 当前层结束，换行
        }
    }

    // 打印叶子节点的数据按顺序排列
// 打印叶子节点的数据按顺序排列，并将每个叶子节点用[]包裹
    void printLeafNodes() {
        Node* current = root;
        // 找到最左边的叶子节点
        while (!current->isLeaf) {
            current = current->children[0];  // 一直走到最左边的叶子节点
        }

        // 逐个叶子节点打印
        while (current != nullptr) {
            std::cout << "[";
            for (int i = 0; i < current->nkey; i++) {
                std::cout << current->keys[i];
                if (i != current->nkey - 1) {
                    std::cout << " ";  // 键之间加空格
                }
            }
            std::cout << "] ";
            current = current->next;  // 移动到下一个叶子节点
        }
        std::cout << std::endl;
    }





    void print() {
        printTreeByLevel();  // 打印每一层的树结构
    }
};
int main() {
    BPlusTree tree;
    std::vector<int> dataList;

    /*
    std::list<int> dataList;
    */
    int num_elements = 1000000;  // 1M データ

       /*
    // 1.1 昇順データの生成
    for (int i = 1; i <= num_elements; ++i) {
        dataList.push_back(i);
    }

           */
   
    /*
    // 1.2 降順データの生成
    for (int i = num_elements; i >= 1; i--) {
        dataList.push_back(i);
    }
       */




    // 1.3 ranndomデータの生成

        // 昇順データを準備
    for (int i = 1; i <= num_elements; ++i) {
        dataList.push_back(i);
    }
    // データをランダム
    std::random_device rd;
    std::mt19937 g(rd());  
    std::shuffle(dataList.begin(), dataList.end(), g);  // データをシャッフル

      



    // 2. B+木へデータinsert
    std::cout << "Inserting data into B+ tree..." << std::endl;
    for (int data : dataList) {
        tree.insert(data);
    }

    std::cout << "All data inserted into B+ tree." << std::endl;

    // 3. データを全て検索
    std::cout << "Searching all inserted data..." << std::endl;
    bool all_found = true;
    for (int data : dataList) {
        if (!tree.search(data)) {
            all_found = false;  // データが見つからない場合
        }
    }

    // 4. 結果
    if (all_found) {
        std::cout << "Yes, all data was found." << std::endl;
    }
    else {
        std::cout << "No, some data was missing." << std::endl;
    }

    return 0;
