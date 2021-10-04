#include <cstdint>
#include <iostream>

template<typename T>
struct IsW1TS {
  static const bool value = false;
};

template<typename T>
struct IsW1TC {
  static const bool value = false;
};

struct RegAddrVal
{
    uint32_t address;
    uint64_t value;
	uint32_t sizeofRegAccess;
};



void WriteReg(uintptr_t const address, uint32_t const data ) {
    *reinterpret_cast<uint32_t*>(address) = data;
    std::cout << "@" << address << " <- " << data << "\n";
}
void ReadReg(uintptr_t const address, uint32_t & data ) {
    data = *reinterpret_cast<uint32_t*>(address);
    std::cout << "@" << address << " -> " << data << "\n";
}

template<typename T> 
void ReadReg(T const volatile & r, uint32_t & readVal ) { 
    if constexpr(IsW1TS<T>::value) {
        readVal = r.raw;
    }  else  if constexpr(IsW1TC<T>::value) {
        readVal = reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(&r)-4)->raw;
    } else {
        readVal = r.raw;
    }
}


template<typename T> 
void WriteReg(T const volatile & r, uint32_t const & val ) {
    if constexpr(IsW1TS<T>::value) {
        uint32_t readval;
        ReadReg(r, readval);
        auto val2write = readval | val;
        WriteReg(reinterpret_cast<uintptr_t>(&r), val2write);
    }  else  if constexpr(IsW1TC<T>::value) {
        uint32_t readval;
        ReadReg(reinterpret_cast<uintptr_t>(&r)-4, readval);
        auto val2write = readval & ~val;
        WriteReg(reinterpret_cast<uintptr_t>(&r)-4, val2write);
    } else {
        std::cout << val << " WriteReg Template\n";
        WriteReg(reinterpret_cast<uintptr_t>(&r), val);
    }
}


template<typename B, typename DataType>
struct RegBlock {
    using datatype = DataType;
    
};
template<typename R, typename B>
    struct RegType {
        using blocktype = B;
        using datatype = typename B::datatype;

        RegAddrVal CastAddrVal()  const volatile {
			R volatile const & r = *static_cast<R const volatile *>(this);
            return RegAddrVal{ reinterpret_cast<uintptr_t>(&r), r, sizeof(datatype) };
        }

        operator datatype() const volatile  {
            R volatile const & r = *static_cast<R const volatile *>(this);
            typename R::datatype val;
            ReadReg(r, val);
            return val;
        }

        operator datatype() const {
            return (static_cast<R const *>(this)->raw);
        }

        void operator=(datatype const val) volatile {
            R volatile& r = *static_cast<R volatile*>(this);
            WriteReg(r, val);
        }

        void operator=(datatype const x) {
            static_cast<R*>(this)->raw = x;
        }
    };

struct Block : public RegBlock<Block, uint32_t>{
struct MyNormalReg : RegType<MyNormalReg, Block> {
    uint32_t raw;
    using RegType<MyNormalReg, Block>::operator=;

    MyNormalReg(uint32_t d = 0) : raw(d) {};

};
 
struct AW1tsReg : RegType<AW1tsReg, Block> {
    uint32_t raw;
    using RegType<AW1tsReg, Block>::operator=;

    AW1tsReg(uint32_t d = 0) : raw(d) {};

};
struct AW1tcReg : RegType<AW1tcReg, Block> {
    uint32_t raw;
    using RegType<AW1tcReg, Block>::operator=;

    AW1tcReg(uint32_t d = 0) : raw(d) {};

};
  MyNormalReg volatile hwActualReg;
  AW1tsReg volatile w1tsReg;
  AW1tcReg volatile w1tcReg;
};

template<>
struct IsW1TS<Block::AW1tsReg> {
  static const bool value = true;
};
template<>
struct IsW1TC<Block::AW1tcReg> {
  static const bool value = true;
};

Block hwBlock;


int main()
{
    std::cout << "running\n";
    uint32_t rv{0};

    hwBlock.hwActualReg = Block::MyNormalReg(10);
    rv = hwBlock.hwActualReg;

    hwBlock.w1tsReg = Block::AW1tsReg(7);
    hwBlock.w1tcReg = Block::AW1tcReg(2);
    std::cout << " hardware w1ts reg: "<< hwBlock.w1tsReg << "\n";
    std::cout << " hardware w1ts reg: "<< hwBlock.w1tcReg << "\n";   
    Block::AW1tsReg tmpW1ts;
    tmpW1ts = 1;
    std::cout << " temp w1ts reg: " << tmpW1ts  << "\n";  ;
    Block::AW1tcReg tmpW1tc;
    tmpW1tc = 1;
    std::cout << " temp w1tc reg: "<< tmpW1tc << "\n";  ;
	return rv;
}


