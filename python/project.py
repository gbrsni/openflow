from inet.simulation.project import *
from inet.common.util import *

openflow_project = define_simulation_project("openflow", version=None,
                                             used_projects=["inet"],
                                             folder_environment_variable="OPENFLOW_ROOT",
                                             library_folder="src",
                                             ned_folders=["src", "scenarios"],
                                             ini_file_folders=["scenarios"],
                                             image_folders=["images"],
                                             fingerprint_store="test/fingerprint/store.json")

openflow_baseline_project = define_simulation_project("openflow-baseline",
                                                      used_projects=["inet-baseline"],
                                                      folder_environment_variable="OPENFLOW_ROOT",
                                                      folder="../openflow-baseline",
                                                      library_folder="src",
                                                      ned_folders=["src", "scenarios"],
                                                      ini_file_folders=["scenarios"],
                                                      image_folders=["images"],
                                                      fingerprint_store="test/fingerprint/store.json") if os.path.exists(get_workspace_path("openflow-baseline")) else None

def testit(**kwargs):
    multiple_results = run_fingerprint_tests(simulation_project=openflow_project, filter="Scenario_USA_ARP_HF_Ping_2C", run_number=0, ingredients_list=["~tNl"], record_eventlog=True, **kwargs)
    baseline_multiple_results = run_fingerprint_tests(simulation_project=openflow_baseline_project, filter="Scenario_USA_ARP_HF_Ping_2C", run_number=0, ingredients_list=["~tNl"], record_eventlog=True, **kwargs)
    fingerprint_trajectory = multiple_results.results[0].get_fingerprint_trajectory().get_unique()
    baseline_fingerprint_trajectory = baseline_multiple_results.results[0].get_fingerprint_trajectory().get_unique()
    return baseline_fingerprint_trajectory.find_divergence_position(fingerprint_trajectory)
