#pragma once 
#include <cstddef>
#include <cstring>
#include <vector>
#include <cstring>
#include <stdint.h>
#include <fmt/core.h>


struct cereal {
  int current_postion = 0;
  std::vector<char> buffer;
  
  cereal(std::vector<char> b): current_postion(0), buffer(b){}
  cereal(const int size): current_postion(0), buffer(size){}
  cereal(char * buf, const int bufSize): current_postion(0), buffer(std::vector<char>(buf,buf+bufSize)) {}  
  
  template <std::size_t N> 
  cereal& peek(int8_t (&arr)[N]){
      std::memcpy(arr,buffer.data()+current_postion,N);
      return *this;
  }
  
  cereal& consume(const int amount){
    current_postion += amount;
    return *this;
  }

  template <typename T>
  cereal& store(char *memBuf, T &val){
    std::memcpy(memBuf+current_postion,&val,sizeof(T));
    current_postion += sizeof(T);
    return *this;
  }  

  template <std::size_t N>
  cereal& store(char *buffer, int8_t (&array)[N]){
    std::memcpy(buffer+current_postion, array,N/sizeof(array[0]));
    current_postion += N;
    return *this;
  }

  template <typename T>
  cereal& store(char* dst, std::vector<T> &src){
    for(auto const &i: src){
      std::memcpy(dst+current_postion,&i,sizeof(T));
      current_postion += sizeof(i);
    }
    return *this;
  }  
  
  template <typename T>
  cereal& store(T &val){
    std::memcpy(buffer.data()+current_postion,&val,sizeof(T));
    current_postion += sizeof(T);
    return *this;
  }  

  template <std::size_t N>
  cereal& store(int8_t (&array)[N]){
    std::memcpy(buffer.data()+current_postion, array,N/sizeof(array[0]));
    current_postion += N;
    return *this;
  }

  template <typename T>
  cereal& store(std::vector<T> &v){
    for(auto const &i: v){
      std::memcpy(buffer.data()+current_postion,&i,sizeof(T));
      current_postion += sizeof(i);
    }
    return *this;
  }  
  
  template <typename T>
  cereal& read(char *buffer, T &val){
    std::memcpy(&val,buffer+current_postion,sizeof(T));
    current_postion += sizeof(val);
    return *this;
  }
  
  template <typename T>
  cereal& read(char *buffer, std::vector<T> &v){
      //TODO find the size to loop through
     for(auto i = 0; i < 2; i++){
      T t{};
      std::memcpy(&t,buffer+current_postion, sizeof(T));
      current_postion += sizeof(T);
      v.push_back(t);
    }
    return *this;
  }

  template <typename T>
  cereal& read(T &val){
    std::memcpy(&val,buffer.data()+current_postion,sizeof(T));
    current_postion += sizeof(val);
    return *this;
  }
  
  template <typename T>
  cereal& read(T &val, const int size){
    
    std::memcpy(&val,buffer.data()+current_postion,size);
    current_postion += sizeof(val);
    return *this;
  }  
  
  template <typename T>
  cereal& read(std::vector<T> &v, const int size){
     const int readTo = size/sizeof(T);

     for(auto i = 0; i < readTo; i++){
      T t{};
      std::memcpy(&t,buffer.data()+current_postion, sizeof(T));
      current_postion += sizeof(T);
      v.push_back(t);
    }
    return *this;
  }  
  
  template <typename T>
  std::vector<T> read(const int size){
     const int readTo = size/sizeof(T);
     std::vector<T> temp(readTo);
     for(auto i = 0; i < readTo; i++){
      T t{};
      std::memcpy(&t,buffer.data()+current_postion, sizeof(T));
      current_postion += sizeof(T);
      temp.push_back(t);
    }
    return temp;
  } 
  void flush(){
    current_postion = 0;
  }

};