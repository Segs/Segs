#include "gtest/gtest.h"
#include "MRubyEngine.h"
class ScriptEngineFixture : public ::testing::Test {
public:
    MRubyEngine *engine;
    virtual void SetUp() {
        engine = new MRubyEngine;
        engine->initialize();
    }
    virtual void TearDown() {
        delete engine;
    }
};
TEST_F(ScriptEngineFixture, string_code_eval) {
    int res = this->engine->eval_code<int>("a=1");
    ASSERT_EQ(res,1);
    res = this->engine->eval_code<int>("1+2");
    ASSERT_EQ(res,3);
    res = this->engine->eval_code<int>("2+2*a");
    ASSERT_EQ(res,4);

}
TEST_F(ScriptEngineFixture, code_eval_exceptions) {
    int res;
    ASSERT_ANY_THROW(res = this->engine->eval_code<int>("'zz'"));
}
TEST_F(ScriptEngineFixture, test_name3) {
    ASSERT_TRUE(false);
}
TEST_F(ScriptEngineFixture, test_name4) {
    ASSERT_TRUE(false);
}
TEST_F(ScriptEngineFixture, test_name5) {
    ASSERT_TRUE(false);
}
