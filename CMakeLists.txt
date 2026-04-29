cmake_minimum_required(VERSION 3.20)

project(ht_planarity_cpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(ht_planarity
    src/Graph.cpp
    src/PlanarityTester.cpp
    src/bcc/BiconnectedComponents.cpp
    src/preprocess/ComponentPreprocessor.cpp
    src/preprocess/PreparedPalmTreeBuilder.cpp
    src/strong/StrongPlanarityTester.cpp
    src/embedding/HTEmbeddingBuilder.cpp
    src/embedding/EmbeddingValidator.cpp
    src/certificate/SegmentForest.cpp
    src/certificate/PathTreeBuilder.cpp
    src/certificate/PathTreeQueries.cpp
    src/certificate/SegmentMetadataBuilder.cpp
    src/certificate/DirectLinkTester.cpp
    src/certificate/KuratowskiCandidateBuilder.cpp
    src/certificate/KuratowskiExtractor.cpp
    src/debug/DebugPrinter.cpp
)

target_include_directories(ht_planarity PUBLIC include)

add_executable(ht_planarity_tests
    tests/TestRunner.cpp
    tests/GraphTest.cpp
    tests/BiconnectedComponentsTest.cpp
    tests/ComponentPreprocessorTest.cpp
    tests/PreparedPalmTreeBuilderTest.cpp
    tests/StrongPlanarityTest.cpp
    tests/EmbeddingValidatorTest.cpp
    tests/PlanarityPipelineTest.cpp
    tests/KuratowskiCertificateTest.cpp
    tests/PathTreeBuilderTest.cpp
    tests/PathTreeQueriesTest.cpp
    tests/KuratowskiCandidateBuilderTest.cpp
    tests/SegmentMetadataBuilderTest.cpp
    tests/DirectLinkTesterTest.cpp
)

target_link_libraries(ht_planarity_tests PRIVATE ht_planarity)

add_executable(ht_failure_witness_debug
    tools/FailureWitnessDebug.cpp
)

target_link_libraries(ht_failure_witness_debug PRIVATE ht_planarity)

enable_testing()
add_test(NAME ht_planarity_tests COMMAND ht_planarity_tests)
