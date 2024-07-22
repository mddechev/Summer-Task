#pragma once 

#include <cctype>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <new>
#include <stdexcept>

template<typename T>
class DataSource {
public:
    virtual ~DataSource() = default;
    
    virtual operator bool() const = 0;
    virtual T operator()() = 0;
    virtual DataSource& operator>>(T& element) = 0;

    //virtual DataSource* clone() const = 0;

    virtual T getElement() = 0;
    virtual T* getElementsInBatch(size_t count) = 0;
    
    virtual bool hasNext() const = 0;
    virtual bool reset() = 0;
};

template<typename T>
class DefaultDataSource: public DataSource<T> {
public:
    operator bool() const override;
    T operator()() override;
    DataSource<T>& operator>>(T& element) override;

    //DataSource<T>* clone() const override;

    T getElement() override;
    T* getElementsInBatch(size_t count) override;

    bool hasNext() const override;
    bool reset() override;
};

template<typename T>
DefaultDataSource<T>::operator bool() const {
    return true;
}

template<typename T>
T DefaultDataSource<T>::operator()() {
    return getElement();
}

template<typename T>
DataSource<T>& DefaultDataSource<T>::operator>>(T& element) {
    element = getElement();
    return *this;
}

// template<typename T>
// DataSource<T>* DefaultDataSource<T>::clone() const {
//     return new DefaultDataSource(*this);
// }

template<typename T>
T DefaultDataSource<T>::getElement() {
    return T();
}

template<typename T>
T* DefaultDataSource<T>::getElementsInBatch(size_t count) {
    T* batch = new T[count];
    for (size_t i = 0; i < count; i++) {
        batch[i] = T();
    }
    return batch;
}

template<typename T>
bool DefaultDataSource<T>::hasNext() const {
    return true;
}

template<typename T>
bool DefaultDataSource<T>::reset() {
    return true;
}


template<typename T>
class FileDataSource: public DataSource<T> {
public:
    explicit FileDataSource(const char* fileName);
    FileDataSource(const FileDataSource<T>& other) = delete;
    ~FileDataSource() override;
    
    FileDataSource& operator=(const FileDataSource<T>& other) = delete;

    operator bool() const override;
    T operator()() override;
    DataSource<T>& operator>>(T& element) override;

    //DataSource<T>* clone() const override;

    T getElement() override;
    T* getElementsInBatch(size_t count) override;

    bool hasNext() const override;
    bool reset() override;
private:
    //void copy(const FileDataSource<T>& other);
    void openFile(const char* fileName);
    void free();
    void setFileName(const char* fileName);

private:
    char* fileName;
    std::ifstream file;
};

template<typename T>
FileDataSource<T>::FileDataSource(const char* fileName)
    :fileName(nullptr) {
    try {
        setFileName(fileName);
        openFile(fileName);

    } catch (const std::bad_alloc& e) {
        free();
        throw;
    } catch (const std::runtime_error& e) {
        free();
        throw;
    } catch (const std::invalid_argument& e) {
        free();
        throw;
    }
}

// template<typename T>
// FileDataSource<T>::FileDataSource(const FileDataSource<T>& other) 
//     :fileName(nullptr) {
    
//     copy(other);
// }

template<typename T>
FileDataSource<T>::~FileDataSource() {
    file.close();
    free();
}

template<typename T>
FileDataSource<T>::operator bool() const {
    return file.is_open() && !file.eof();
}

template<typename T>
T FileDataSource<T>::operator()() {
    return getElement();
}

template<typename T>
DataSource<T>& FileDataSource<T>::operator>>(T &element) {
    element = getElement();
    return *this;
}

// template<typename T>
// DataSource<T>* FileDataSource<T>::clone() const {
//     return new FileDataSource(*this);
// }

template<typename T>
T FileDataSource<T>::getElement() {
    if (!hasNext()) {
        throw std::runtime_error("No more data");
    }
    T element;
    file >> element;
    if (!file) {
        throw std::runtime_error("No more data");
    }
    return element;
}

template<typename T>
T* FileDataSource<T>::getElementsInBatch(size_t count) {
    if (!hasNext()) {
        throw std::runtime_error("No more data");
    }
    T* batch = new T[count];
    size_t outBatchCount = 0;
    for (size_t i = 0; i < count && file; i++) {
        file >> batch[outBatchCount];
        if (file) {
            ++outBatchCount;
        }
    }
    return batch;
}

template<typename T>
bool FileDataSource<T>::hasNext() const {
    return !file.eof();
}

template<typename T>
bool FileDataSource<T>::reset() {
    file.clear();
    file.seekg(0, std::ios::beg);
    return file.is_open();
}

// template<typename T>
// void FileDataSource<T>::copy(const FileDataSource<T>& other) {
//     setFileName(other.fileName);
//     this->file(other.file);
// }

template<typename T>
void FileDataSource<T>::openFile(const char* fileName) {
    if (!fileName) {
        throw std::invalid_argument("File name cannot be nullptr");
    }

    file.open(fileName);
    if (!file) {
        throw std::runtime_error("Couldn't open file");
    }
}

template<typename T>
void FileDataSource<T>::free() {
    delete [] fileName;
    fileName = nullptr;
}

template<typename T>
void FileDataSource<T>::setFileName(const char* fileName) {
    if (!fileName) {
        throw std::invalid_argument("File name cannot be nullptr");
    }
    this->fileName = new char[strlen(fileName) + 1];
    if (!this->fileName) {
        throw std::bad_alloc();
    }
    strcpy(this->fileName, fileName);
}

template<typename T>
class ArrayDataSource: public DataSource<T> {
public:
    explicit ArrayDataSource(T* arrayData, size_t arraySize);
    ArrayDataSource(const ArrayDataSource<T>&);
    ~ArrayDataSource() override;

    ArrayDataSource& operator=(const ArrayDataSource<T> &);
    ArrayDataSource& operator+=(const T& value);
    ArrayDataSource& operator+(const T& value);
    ArrayDataSource& operator--(); //prefix;
    ArrayDataSource& operator--(int); //postfix;
    operator bool() const override;
    T operator()() override;
    DataSource<T>& operator>>(T& element) override;

    T getElement() override;
    T* getElementsInBatch(size_t count) override;

    // size_t getSize() const { return size; }
    // size_t getCapacity() const { return capacity; }
    // size_t getCurrentPosition() const { return  currentPos; }

    bool hasNext() const override;
    bool reset() override;

private:
    void copy(const ArrayDataSource<T>& other);
    void resize(size_t step = RESIZE_STEP);
    void reserve(size_t capacity);
    void free();

private:
    static const size_t STARTING_POS = 0;
    static const size_t RESIZE_STEP = 2;
private:
    T* data;
    size_t size;
    size_t capacity;
    size_t currentPos;
};

template<typename T>
ArrayDataSource<T>::ArrayDataSource(T* arrayData, size_t arraySize)
    :data(nullptr), size(arraySize), capacity(arraySize), currentPos(STARTING_POS) {
    try {
        reserve(capacity);
        for (size_t i = 0; i < arraySize ; i++) {
            data[i] = arrayData[i];
        }
    } catch (std::bad_alloc& e){
        free();
        throw;
    } catch (std::invalid_argument& e) {
        free();
        throw;
    }
}

template<typename T>
ArrayDataSource<T>::ArrayDataSource(const ArrayDataSource<T>& other)    
    :data(nullptr) {
    copy(other);
}

template<typename T>
ArrayDataSource<T>::~ArrayDataSource<T>() {
    free();
}

template<typename T>
ArrayDataSource<T>& ArrayDataSource<T>::operator=(const ArrayDataSource<T>& other) {
    if (this != &other) {
        free();
        copy(other);
    }
    return *this;
}

template<typename T>
ArrayDataSource<T>& ArrayDataSource<T>::operator+=(const T &value) {
    if (size >= capacity) {
        resize();
    }
    data[size++] = value;
    return *this;
}

template<typename T>
ArrayDataSource<T>& ArrayDataSource<T>::operator+(const T &value) {
    if (size >= capacity) {
        resize();
    }
    data[size++] = value;
    return *this;
}

template<typename T>
ArrayDataSource<T>& ArrayDataSource<T>::operator--() {
    if (currentPos > STARTING_POS) {
        --currentPos;
    }
    return *this;
}

template<typename T>
ArrayDataSource<T>& ArrayDataSource<T>::operator--(int) {
    ArrayDataSource temp = *this;
    --(*this);
    return temp;
}

template<typename T>
ArrayDataSource<T>::operator bool() const {
    return currentPos < size;
}

template<typename T>
T ArrayDataSource<T>::operator()() {
    return getElement();
}

template<typename T>
DataSource<T>& ArrayDataSource<T>::operator>>(T& element) {
    element = getElement();
    return *this;
}

template<typename T>
T ArrayDataSource<T>::getElement() {
    // if (currentPos >= size) {
    //     throw std::runtime_error("No more data");
    // }
    // if (!hasNext()) {
    //     throw std::runtime_error("No more data(No next)");
    // }
    // return data[currentPos++];
    if (currentPos < size) {
        return data[currentPos++];
    } else {
        throw std::out_of_range("No more elements");
    }
}

template<typename T>
T* ArrayDataSource<T>::getElementsInBatch(size_t count) {
    // size_t remaining = size - currentPos;
    // size_t outBatchCount = (count < remaining) ? count : remaining;
    // T* batch = new T[outBatchCount];
    // for (size_t i = 0; i < outBatchCount; i++) {
    //     batch[i] = data[currentPos++];
    // }
    // return batch;
    if (currentPos + count > size) {
        count = size - currentPos;
    }
    T* batch = new T[count];
    for (size_t i = 0; i < count; i++) {
        batch[i] = data[currentPos++];
    }
    return batch;
}

template<typename T>
bool ArrayDataSource<T>::hasNext() const {
    if (currentPos == size) {
        return false;
    }
    return currentPos < size;
}

template<typename T>
bool ArrayDataSource<T>::reset() {
    currentPos = STARTING_POS;
    return true;
}

template<typename T>
void ArrayDataSource<T>::copy(const ArrayDataSource<T>& other) {
    this->size = other.size;
    this->capacity = other.capacity;
    this->currentPos = other.currentPos;
    reserve(other.capacity);
    for (size_t i = 0; i < other.size; i++) {
        this->data[i] = other.data[i];
    }
}

template<typename T>
void ArrayDataSource<T>::resize(size_t step) {
    size_t newCap = capacity * step;
    T* newData = new T[newCap];

    for (size_t i = 0; i < size; i++) {
        newData[i] = data[i];
    }
    delete [] data;
    data = newData;
    capacity = newCap;

}

template<typename T>
void ArrayDataSource<T>::reserve(size_t capacity) {
    data = new T[capacity];
    if (!data) {
        throw std::bad_alloc();
    }
}

template<typename T>
void ArrayDataSource<T>::free() {
    delete [] data;
    data = nullptr;
}

template<typename T>
class AlternateDataSource: public DataSource<T> {
public:
    AlternateDataSource(DataSource<T>** srcs, size_t count);
    
    operator bool() const override;
    T operator()() override;
    DataSource<T>& operator>>(T& elem) override;

    T getElement() override;
    T* getElementsInBatch(size_t count);

    bool hasNext() const override;
    bool reset();
private:
    void reserve(size_t capacity);
    void free();

private:
    static const size_t STARTING_SOURCE_INDEX = 0;
private:
    DataSource<T>** sources;
    size_t sourcesCount;
    size_t currentSourceIndex;
};

template<typename T>
AlternateDataSource<T>::AlternateDataSource(DataSource<T>** srcs, size_t count) 
    :sources(nullptr), sourcesCount(count), currentSourceIndex(STARTING_SOURCE_INDEX){
    try {
        reserve(sourcesCount);

    } catch (const std::bad_alloc& e) {
        free();
        throw;
    }
}

template<typename T>
AlternateDataSource<T>::operator bool() const {
    return hasNext();
}

template<typename T>
DataSource<T>& AlternateDataSource<T>::operator>>(T &elem) {

}

template<typename T>
T AlternateDataSource<T>::getElement() {
    for (size_t i = 0; i < sourcesCount; i++) {
        if (sources[currentSourceIndex]->hasNext()) {
            return sources[currentSourceIndex++]->getElement();
        }
        currentSourceIndex = (currentSourceIndex + 1) % sourcesCount;
    }
    throw std::out_of_range("No more elements");
}

// template<typename T>
// template<typename T>
// template<typename T>
// template<typename T>
// template<typename T>
// template<typename T>


