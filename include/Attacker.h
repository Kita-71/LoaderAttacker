#ifndef _ATTACKER_H_
#define _ATTACKER_H_
#include <Windows.h>

#include "ImgManager.h"
typedef void (*pHello)();
class Attacker {
 public:
  // 删除拷贝构造函数和赋值操作符，确保Singleton不可被复制
  Attacker(const Attacker&) = delete;
  Attacker& operator=(const Attacker&) = delete;
  static Attacker& getInstance(ImgManager* imgManager) {
    static Attacker instance(imgManager);
    return instance;
  }

 private:
  ImgManager* _imgManager;
  // 私有化构造函数，确保只能内部创建实例
  explicit Attacker(ImgManager* imgManager) : _imgManager(imgManager) {}

  void PatchFunction(BYTE* dst, void* hook);

 public:
  pHello originalHello;
  BYTE originalBytes[5];
  void UnpatchFunction(BYTE* dst);
  void InstallHook();
  virtual ~Attacker(){};
  void Attack();
};

#endif