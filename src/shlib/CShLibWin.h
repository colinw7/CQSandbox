class CShLibWin : public CShLibImpl {
 public:
  CShLibWin(const std::string &filename);
 ~CShLibWin();

  bool open() override;
  bool getProc(const std::string &name, CShLibProc *proc) override;
  bool getData(const std::string &name, CShLibData *data) override;
  void close() override;
};
