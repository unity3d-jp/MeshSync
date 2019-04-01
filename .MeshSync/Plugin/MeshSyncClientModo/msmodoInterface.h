#pragma once

class msmodoInterface
{
public:
    void prepare();

    // time: inf -> current time
    void setChannelReadTime(double time = std::numeric_limits<double>::infinity());

    void enumerateItemGraphR(CLxUser_Item& item, const char *graph_name, const std::function<void(CLxUser_Item&)>& body);
    void enumerateItemGraphF(CLxUser_Item& item, const char *graph_name, const std::function<void(CLxUser_Item&)>& body);

    void enumerateChannelGraphR(CLxUser_Item& item, int channel, const char *graph_name, const std::function<void(CLxUser_Item&)>& body);

    void eachMaterial(const std::function<void(CLxUser_Item&)>& body);
    void eachLight(const std::function<void(CLxUser_Item&)>& body);
    void eachCamera(const std::function<void(CLxUser_Item&)>& body);
    void eachMesh(const std::function<void(CLxUser_Item&)>& body);
    void eachMeshInstance(const std::function<void(CLxUser_Item&)>& body);
    void eachDeformer(const std::function<void(CLxUser_Item&)>& body);
    void eachMesh(CLxUser_Item& deformer, const std::function<void(CLxUser_Item&)>& body);
    void eachBone(CLxUser_Item& item, const std::function<void(CLxUser_Item&)>& body);
    CLxUser_Mesh getBaseMesh(CLxUser_Item& obj);
    CLxUser_Mesh getDeformedMesh(CLxUser_Item& obj);

    std::tuple<double, double> getTimeRange();

    void dumpChannels(CLxUser_Item item);

public:
    LXtItemType tMaterial = 0,
                tLocator, tCamera, tLight, tMesh, tMeshInst,
                tLightMaterial, tPointLight, tDirectionalLight, tSpotLight, tAreaLight,
                tDeform, tGenInf, tMorph;

protected:
    CLxUser_SceneService     m_scene_service;
    CLxUser_SelectionService m_selection_service;
    CLxUser_LayerService     m_layer_service;
    CLxUser_MeshService      m_mesh_service;
    CLxUser_DeformerService  m_deform_service;
    CLxUser_LogService       m_log_service;

    CLxUser_Scene m_current_scene;
    CLxUser_ChannelRead m_ch_read;
    CLxUser_ChannelRead m_ch_read_setup;

private:
};
