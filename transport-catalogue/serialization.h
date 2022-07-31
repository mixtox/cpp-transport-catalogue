#pragma once

#include <filesystem>

#include "domain.h"
#include "json_reader.h"
#include "transport_catalogue.h"
#include "map_renderer.h"

#include <transport_catalogue.pb.h>
#include <transport_router.pb.h>
#include <svg.pb.h>
#include <map_renderer.pb.h>

#include <fstream>
#include <sstream>
#include <ostream>
#include <iostream>

namespace serialization {

    class Serialization {
    public:
        explicit Serialization(transport_catalogue::TransportCatalogue &catalogue,
                               const json_reader::JsonReader &input,
                               renderer::MapRenderer &renderer,
                               transport_router::TransportRouter &router);

        void SaveDataBase();
        void LoadDataBase();

    private:
        transport_catalogue::TransportCatalogue &tc_;
        const json_reader::JsonReader &jr_;
        renderer::MapRenderer &mr_;
        transport_router::TransportRouter &tr_;

        mutable tc_serialize::TC data_base_;

        tc_serialize::Stop SaveStop(const domain::Stop &stop) const;
        tc_serialize::Bus SaveBus(const domain::Bus &buse) const;
        tc_serialize::DistanceBetweenStop
        SaveDistance(domain::StopPtr from, domain::StopPtr to, double distance) const;

        void SaveStops();
        void SaveBuses();
        void SaveDistance();

        svg_proto::Point SavePoint(const svg::Point &point);
        svg_proto::Color SaveColor(const svg::Color &color);
        svg_proto::RGB SaveRGB(const svg::Rgb &rgb);
        svg_proto::RGBA SaveRGBa(const svg::Rgba &rgba);

        void SaveRenderSettings();
        void SaveRouteSetting();

        void SaveGraphs();
        void SaveGraphVertex();
        void SaveGraphEdge();
        void SaveGraph();

        void LoadStop(const tc_serialize::Stop &stop);
        void LoadBus(const tc_serialize::Bus &bus);
        void LoadDistance(const tc_serialize::DistanceBetweenStop &distance);

        void LoadStops();
        void LoadBuses();
        void LoadDistance();

        void LoadRenderSettings();

        svg::Point LoadPoint(const svg_proto::Point &point);
        svg::Color LoadColor(const svg_proto::Color &color);
        svg::Rgb LoadRGB(const svg_proto::RGB &rgb);
        svg::Rgba LoadRGBA(const svg_proto::RGBA &rgba);

        void LoadRouteSetting();

        void LoadGraphs();
        void LoadGraphVertex();
        void LoadGraphEdge();
        void LoadGraph();

    };

}
