#include <Arduino.h>

bool isNumeric(String str){
   for(byte i = 0; i < str.length(); i++){
      if(isDigit(str.charAt(i))) return true;
   }
   return false;
}

bool isNumeric(const char* str){
   for(byte i = 0; i < strlen(str); i++){
      if(isDigit(str[i])) return true;
   }
   return false;
}

int indexOf(int value, int* arr, int size){
   for (int i = 0; i < size; i++)
   {
      if(value == arr[i]){
         return i;
      }
   }
   return -1;
}

int indexOf(int value, unsigned int* arr, int size){
   for (size_t i = 0; i < size; i++)
   {
      if(value == arr[i]){
         return i;
      }
   }
   return -1;
}

int getMinIndex(int* arr, int size){
   int min = arr[0];
   int minIndex = 0;
   for (int i = 0; i < size; i++)
   {
      if(min > arr[i]){
         minIndex = i;
         min = arr[i];
      }
   }
   return minIndex;
}

int getMaxIndex(int* arr, int size){
   int max = arr[0];
   int maxIndex = 0;
   for (int i = 0; i < size; i++)
   {
      if(max < arr[i]){
         maxIndex = i;
         max = arr[i];
      }
   }
   return maxIndex;
}

char* getNewCharFromInt(int num){
   char* out = new char[10];
   sprintf(out, "%i", num);
   return out;
}