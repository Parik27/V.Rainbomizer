#pragma once

class fiAssetMgr
{
public:

  inline static fiAssetMgr* sm_Instance = nullptr;

  void PushFolder (const char *folder);
  void PopFolder ();

  static void InitialisePatterns ();
};
