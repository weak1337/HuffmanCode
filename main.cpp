#include <iostream>
#include <string>
#include <queue>
#include <unordered_map>
#include <cmath>

struct node{
  char character;
  int frequency;

  node* left, * right;
  node(char c, int freq, node* _left, node* _right){
      character = c;
      frequency = freq;
      left = _left;
      right = _right;
  }
};

struct compare{
    bool operator()(node* left, node* right) {
        return left->frequency > right->frequency;
    }
};



void build_encoding_table(node* root, std::string text, std::unordered_map<char, std::string>&table){
    if(root == nullptr)
        return;
    if(root->left == nullptr && root->right == nullptr){
        table[root->character] = text;
        return;
    }
    else{
        build_encoding_table(root->left, text + "0", table);
        build_encoding_table(root->right, text + "1", table);
        return;
    }
}


void save_huffman_tree(node* root, char* data){
    static int offset = 0;
  
    if(root->right == nullptr && root->left == nullptr){
        data[offset] = 1;
        data[offset + 1] = root->character;
        offset+=2;
        
        return;
    }else{
        data[offset] = 0;
        offset++;
        save_huffman_tree(root->left, data);
        save_huffman_tree(root->right, data);
    }
    
}

node* extract_huffman_tree(char* data){
      static int offset2 = 0;
      if(data[offset2] == 1){
          offset2+=2;
          return new node(data[offset2 - 1],0,nullptr, nullptr);
      }
      else{
          offset2++;
          node* left = extract_huffman_tree(data);
          node* right = extract_huffman_tree(data);
          return new node(0,0,left,right);
      }
}

int get_node_count(node* root){
    if(root == nullptr)
        return 0;
    return 1 + get_node_count(root->left) + get_node_count(root->right);
}

void decode(node* root, int&index, char* data){
    if(root == nullptr)
        return;
    if(root->left == nullptr && root->right == nullptr){
        printf("%c", root->character);
        return;
    }
    index++;
    int corresponding_byte = ceil(index / 8);
    int corresponding_bit = index - corresponding_byte * 8;
    char* byte_to_check = (char*)((long long)data + corresponding_byte);
    if((*byte_to_check) & 1 << corresponding_bit )
        decode(root->right, index, data);
    else
        decode(root->left, index, data); 
}


int main() {
    std::string text = "This is a text that we want to compress!";
    printf("Compressing: '%s' with length of %i\n", text.c_str(), text.size());
    
    std::unordered_map<char, int>characters;
    for(char c : text)
        characters[c]++;


   

    std::priority_queue<node*, std::vector<node*>,compare>queue; 
    
    for(auto pair : characters){
        queue.push(new node(pair.first, pair.second, nullptr, nullptr));
         printf("Found character %c with frequency of: %i\n", pair.first, pair.second);
    }
 
    while(queue.size() != 1){
        node* left = queue.top(); queue.pop();
        node* right = queue.top(); queue.pop();
        int freq_sum = left->frequency + right->frequency;
        queue.push(new node(0,freq_sum,left, right));
    }
    
    node* root = queue.top();
    std::unordered_map<char, std::string>encodingtable;
    build_encoding_table(root, "", encodingtable);
    
    printf("Encoding Table:\n");
    for(auto pair : encodingtable){
        printf("%c -> %s\n", pair.first, pair.second.c_str());
    }
    
    int required_bits = 0;
    for(auto c : text)
    {
        required_bits += encodingtable[c].size();
    }
    printf("Required bits for plain-text: %i and compressed-text: %i\n",text.size() * 8, required_bits);
    
    int bits_used = 0;
    char* bitmap = (char*)malloc(required_bits);
    std::string completestring = "";
    for(auto c : text){
        std::string bit_representation = encodingtable[c];
        completestring += bit_representation;
        for(int i = 0; i < bit_representation.size(); i++){
            int corresponding_byte = ceil(bits_used / 8);
            int corresponding_bit = bits_used - corresponding_byte * 8;
            char cbit_flip = bit_representation.at(i);
            int bit_state = std::atoi(&cbit_flip);
            
            char* byte_to_modify = (char*)((long long)bitmap + corresponding_byte);
            *byte_to_modify |= bit_state << corresponding_bit;
            bits_used++;
        }
    }
    
    //Save Huffman Tree
    int node_count = get_node_count(root);
    char* huffman_tree_data = (char*)malloc(node_count * 2); //This could be optimized
    save_huffman_tree(root, huffman_tree_data);
    
    //Extract Huffman Tree
    node* huffman_tree = extract_huffman_tree(huffman_tree_data);
    int node_count2 = get_node_count(huffman_tree);
   
   printf("Node count before saving %i after %i\n", node_count, node_count2);

    int index = -1;
    while(index < (int)required_bits - 2)
    {
        decode(huffman_tree, index, bitmap);
    }
    
    printf("\nResults: [%f] Original: %i bits Compressed+Tree %i bits\n", 
    ((float)(required_bits + node_count * 2 ) / (float)(text.size() * 8) ) * 100.f,
    text.size() * 8, 
    required_bits + node_count * 2);
    
    return 0;
}
